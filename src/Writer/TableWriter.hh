<?hh // strict
namespace Usox\HaRm\Writer;

use Usox\HaRm\Generator\HarmGenerator;
use Usox\HaRm\Generator\DbAttribute;
use Facebook\HackCodegen\HackCodegenFactory;
use Facebook\HackCodegen\HackCodegenConfig;
use Facebook\HackCodegen\CodegenFileType;
use Facebook\HackCodegen\CodegenFile;
use Facebook\HackCodegen\CodegenClass;

final class TableWriter {

	private CodegenFile $file;
	private CodegenClass $class;

	private HackCodegenFactory $cg_factory;

	public function __construct(private HarmGenerator $harm): void {
		$this->cg_factory = new HackCodegenFactory(
			new HackCodegenConfig()
		);
		
		$this->file = $this->cg_factory
			->codegenFile(
				sprintf('%s.hh', $this->harm->getClassName())
			)
			->setIsStrict(true)
			->setNamespace($this->harm->getNamespaceName())
			->useNamespace('Usox\HaDb\DatabaseInterface')
			->setDoClobber(true);

		$this->class = $this->cg_factory
			->codegenClass(
				$this->harm->getClassName()
			)
			->setIsFinal(true)
			->addInterface(
				$this->cg_factory
				->codegenImplementsInterface(
					sprintf('%sInterface', $this->harm->getClassName())
				)
			)
			->addVar(
				$this->cg_factory
					->codegenMemberVar($this->harm->getPrimaryKeyName())
					->setType('int')
					->setValue(0)
			)
			->addVar(
				$this->cg_factory
					->codegenMemberVar('data_loaded')
					->setType('bool')
					->setValue(false)
			)
			->addVar(
				$this->cg_factory
					->codegenMemberVar('modified')
					->setType('bool')
					->setValue(false)
			)
			->addVar(
				$this->cg_factory
					->codegenMemberVar('dirty')
					->setType('Map<string, bool>')
			);

	}

	public function writeOut(): void {
		foreach ($this->harm->getAttributes() as $attribute) {
			$var = $this->cg_factory
				->codegenMemberVar($attribute->getName())
				->setType($attribute->getWriteTypeHint());

			$attribute->setDefaultValue($var);
			$this->class->addVar($var);
		}

		$this->writeConstructor();
		$this->writeIsNew();
		$this->writeLoadDataByDatabaseResult();
		$this->writeSave();
		$this->writeGetTableName();
		$this->writeAttributeAccessors();
		$this->writeDelete();
		$this->writeCount();
		$this->writeExists();
		$this->writeFindObject();
		$this->writeQuote();
		$this->writeGetObjectsBy();
		$this->writeGetById();
		$this->writeGetDirtyForWrite();
		$this->writeUpdate();
		$this->writeInsert();
		$this->writeStartDirtyTaggingWriter();
		$this->writeFooter();

		$this->file->addClass($this->class);
		echo $this->file->render();
	}

	private function writeStartDirtyTaggingWriter(): void {
		$attributes = $this->harm->getAttributes();
		$attribute_dirty_tags = '';

		foreach ($attributes as $attribute) {
			$attribute_dirty_tags .= sprintf('\'%s\' => false,', $attribute->getName());
		}

		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('startDirtyTagging')
				->setReturnType('void')
				->setPrivate()
				->setBodyf(
					'$this->dirty = new Map([%s]);',
					$attribute_dirty_tags
				)
		);
	}

	private function writeInsert(): void {
		$keyname = $this->harm->getPrimaryKeyName();

		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('insert')
				->setReturnType('void')
				->setPrivate()
				->setBodyf(
					"%s\n%s\n\t%s\n%s\n%s\n%s\n%s\n%s",
					'$dirty_for_write = $this->getDirtyForWrite();',
					sprintf('if ($this->%s != 0) {', $keyname),
					sprintf('$dirty_for_write->add(Pair{\'%s\', (string) $this->%s});', $keyname, $keyname),
					'}',
					sprintf(
						'$this->database->query(\'INSERT INTO %s (\'.implode(\', \', $dirty_for_write->keys()).\') VALUES (\\\'\'.implode(\'\\\', \\\'\', $dirty_for_write->values()).\'\\\')\');',
						$this->harm->getTableName(),
					),
					'$this->id = $this->database->getLastInsertedId();',
					'$this->data_loaded = true;',
					'$this->startDirtyTagging();'
				)
		);
	}

	private function writeUpdate(): void {
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('update')
				->setReturnType('void')
				->setPrivate()
				->setBodyf(
					"%s\n\t%s\n%s\n%s\n%s\n\t%s\n%s\n%s\n\t%s\n%s\n%s",
					'if (!$this->isModified()) {',
					'return;',
					'}',
					'$attribute_cast_list = Vector{};',
					'foreach ($this->getDirtyForWrite() as $field => $value) {',
					'$attribute_cast_list[] = $field.\' = \\\'\'.(string) $value.\'\\\'\';',
					'}',
					'if ($attribute_cast_list->count() === 0) {',
					'return;',
					'}',
					sprintf(
						'$this->database->query(\'UPDATE %s SET \'.implode(\', \', $attribute_cast_list).\' WHERE %s = \'.$this->getId());',
						$this->harm->getTableName(),
						$this->harm->getPrimaryKeyName(),
					)
				)
		);
	}

	private function writeGetDirtyForWrite(): void {
		$body = '';
		
		foreach ($this->harm->getAttributes() as $attribute) {
			$name = $attribute->getName();
			$write_cast = $attribute->getWriteCast('$this->'.$name);
			$body .= 'if ($this->isDirty(\''.$name.'\')) {'."\n";
			$body .= "\t".'$attributes[\''.$name.'\'] = $this->'.$name.' === null ? \'null\' : '.$write_cast.';'."\n";
			$body .= '}'."\n";
		}

		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('getDirtyForWrite')
				->setReturnType('Map<string, string>')
				->setPrivate()
				->setBodyf(
					"%s\n%s\n%s",
					'$attributes = Map{};',
					$body,
					'return $attributes;'
				)
		);
	}

	private function writeGetById(): void {
		$attribute_list = Vector{};
		$attribute_list[] = $this->harm->getPrimaryKeyName();
		foreach ($this->harm->getAttributes() as $attribute) {
			$attribute_list[] = $attribute->getDBReadCast();
		}
		
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('getById')
				->addParameter('int $id')
				->setReturnType(sprintf('?%sInterface', $this->harm->getClassName()))
				->setBodyf(
					"%s\n%s\n%s\n%s\n\t%s\n%s\n%s\n%s",
					sprintf(
						'$query = \'SELECT %s FROM %s WHERE %s = \'.$id;',
						implode(',', $attribute_list),
						$this->harm->getTableName(),
						$this->harm->getPrimaryKeyName(),
					),
					'$object = new self($this->database);',
					'$data = $this->database->getNextResult($this->database->query($query));',
					'if ($data === null) {',
					'return null;',
					'}',
					'$object->loadDataByDatabaseResult($data);',
					'return $object;'
				)
		);
	}

	private function writeGetObjectsBy(): void {

		$attribute_list = Vector{};
		$attribute_list[] = $this->harm->getPrimaryKeyName();
		foreach ($this->harm->getAttributes() as $attribute) {
			$attribute_list[] = $attribute->getDBReadCast();
		}
		
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('getObjectsBy')
				->addParameter('?string $condition = null')
				->addParameter('?string $order = null')
				->addParameter('?string $addendum = null')
				->setReturnType(sprintf('Vector<%sInterface>', $this->harm->getClassName()))
				->setBodyf(
					"%s\n%s\n%s\n%s\n%s\n%s\n%s\n\t%s\n\t%s\n\t%s\n%s\n%s",
					sprintf('$query = \'SELECT %s FROM %s\';', implode(',', $attribute_list), $this->harm->getTableName()),
					'if ($condition !== null) $query .= \' WHERE \'.$condition;',
					'if ($order !== null) $query .= \' ORDER BY \'.$order;',
					'if ($addendum !== null) $query .= \' \'.$addendum;',
					'$query_result = $this->database->query($query);',
					'$results = Vector{};',
					'while ($result = $this->database->getNextResult($query_result)) {',
					'$obj = new self($this->database);',
					'$obj->loadDataByDatabaseResult($result);',
					'$results->add($obj);',
					'}',
					'return $results;',
				)
		);
	}

	private function writeFindObject(): void {
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('findObject')
				->addParameter('?string $condition = null')
				->setReturnType(sprintf('%sInterface', $this->harm->getClassName()))
				->setBodyf(
					"%s\n%s\n\t%s\n%s\n%s",
					'$iterator = $this->getObjectsBy($condition, null, \'LIMIT 1\');',
					'if ($iterator->count() === 0) {',
					'throw new \Usox\HaRm\Exception\ObjectNotFoundException();',
					'}',
					'return $iterator->items()->getIterator()->current();'
				)
		);
	}

	private function writeQuote(): void {
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('quote')
				->addParameter('string $subject')
				->setReturnType('string')
				->setBody(
					'return $this->database->quote($subject);'
				)
		);
	}

	private function writeCount(): void {
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('count')
				->addParameter('?string $condition = null')
				->setReturnType('int')
				->setBodyf(
					"%s\n%s",
					'$condition !== null ? $condition = sprintf(\'WHERE %s\', $condition) : $condition = \'\';',
					sprintf('return $this->database->count(sprintf(\'SELECT COUNT(%s) as count FROM %%s %%s\', $this->getTableName(), $condition));', $this->harm->getPrimaryKeyName()),
				)
		);
	}

	private function writeExists(): void {
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('exists')
				->addParameter('?string $condition = null')
				->setReturnType('bool')
				->setBodyf(
					"%s\n%s",
					'$condition !== null ? $condition = sprintf(\'WHERE %s\', $condition) : $condition = \'\';',
					'return $this->database->exists(sprintf(\'SELECT 1 FROM %s %s\', $this->getTableName(), $condition));',
				)
		);
	}

	private function writeDelete(): void {
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('delete')
				->setReturnType('void')
				->setBodyf(
					'$this->database->query(\'DELETE FROM %s WHERE %s = \'.$this->getId());',
					$this->harm->getTableName(),
					$this->harm->getPrimaryKeyName()
				)
		);
	}

	private function writeAttributeAccessors(): void {
		$attributes = $this->harm->getAttributes();
		$attributes->add(new DbAttribute($this->harm->getPrimaryKeyName(), 'int'));
		foreach ($attributes as $attribute) {
			$accessor_name = $attribute->getAccessorName();
			$attribute_name = $attribute->getName();

			$this->class->addMethod(
				$this->cg_factory
					->codegenMethod(sprintf('get%s', $accessor_name))
					->setReturnType($attribute->getWriteTypeHint())
					->setBodyf(
						'return $this->%s;',
						$attribute_name
					)
			);
			$this->class->addMethod(
				$this->cg_factory
					->codegenMethod(sprintf('set%s', $accessor_name))
					->addParameter(sprintf('%s $value', $attribute->getWriteTypeHint()))
					->setReturnType('void')
					->setBodyf(
						"%s\n%s",
						sprintf('$this->tagDirty(\'%s\');', $attribute_name),
						sprintf('$this->%s = $value;', $attribute_name)
					)
			);
		}
	}

	private function writeSave(): void {
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('save')
				->setReturnType('void')
				->setBodyf(
					"%s\n\t%s\n%s\n\t%s\n%s",
					'if ($this->isNew()) {',
					'$this->insert();',
					'} else {',
					'$this->update();',
					'}'
				)
		);
	}

	private function writeIsNew(): void {
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('isNew')
				->setReturnType('bool')
				->setBody('return $this->data_loaded !== true;')
		);
	}

	private function writeGetTableName(): void {
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('getTableName')
				->setReturnType('string')
				->setBodyf(
					'return \'%s\';',
					$this->harm->getTableName()
				)
		);
	}

	private function writeConstructor(): void {
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('__construct')
				->addParameter('private DatabaseInterface $database')
				->setReturnType('void')
				->setBody('$this->startDirtyTagging();')
		);
	}

	private function writeLoadDataByDatabaseResult(): void {
		$keyname = $this->harm->getPrimaryKeyName();
		$body = '';

		foreach ($this->harm->getAttributes() as $attribute) {
			$body .= sprintf(
				'$this->set%s((%s) %s);'."\n",
				$attribute->getAccessorName(),
				$attribute->getWriteTypeHint(),
				$attribute->getReadCast('$data[\''.$attribute->getName().'\']')
			);
		}

		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('loadDataByDatabaseResult')
				->addParameter('array<string, ?string> $data')
				->setReturnType('void')
				->setBodyf(
					"%s\n%s\n%s\n%s\n%s",
					'$this->data_loaded = true;',
					'$this->modified = false;',
					sprintf('$this->%s = (int) $data[\'%s\'];', $keyname, $keyname),
					$body,
					'$this->startDirtyTagging();'
				)
		);
	}

	private function writeFooter(): void {
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('tagDirty')
				->addParameter('string $attribute')
				->setReturnType('void')
				->setPrivate()
				->setBodyf(
					"%s\n%s",
					'$this->dirty[$attribute] = true;',
					'$this->modified = true;'
				)
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('isDirty')
				->addParameter('string $attribute')
				->setReturnType('bool')
				->setPrivate()
				->setBody('return $this->dirty[$attribute] === true;')
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('isModified')
				->setReturnType('bool')
				->setPrivate()
				->setBody('return $this->modified;')
		);
	}
}
