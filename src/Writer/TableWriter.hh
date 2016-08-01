<?hh // strict
namespace Usox\HaRm\Writer;

use Usox\HaRm\Generator\HarmGenerator;
use Usox\HaRm\Generator\DbAttribute;
use Facebook\HackCodegen as codegen;

final class TableWriter {

	private codegen\CodegenFile $file;
	private codegen\CodegenClass $class;

	public function __construct(private HarmGenerator $harm): void {
		$this->file = codegen\codegen_file(
				sprintf('%s.hh', $this->harm->getClassName())
			)
			->setIsStrict(true)
			->setNamespace($this->harm->getNamespaceName())
			->useNamespace('Usox\HaDb\DatabaseInterface');

		$this->class = codegen\codegen_class($this->harm->getClassName())
			->setIsFinal(true)
			->addInterface(
				codegen\codegen_implements_interface(
					sprintf('%sInterface', $this->harm->getClassName())
				)
			)
			->addVar(
				codegen\codegen_member_var($this->harm->getPrimaryKeyName())
				->setType('int')
				->setValue(0)
			)
			->addVar(
				codegen\codegen_member_var('data_loaded')
				->setType('bool')
				->setValue(false)
			)
			->addVar(
				codegen\codegen_member_var('modified')
				->setType('bool')
				->setValue(false)
			)
			->addVar(
				codegen\codegen_member_var('dirty')
				->setType('Map<string, bool>')
			);
	}

	public function writeOut(): void {
		foreach ($this->harm->getAttributes() as $attribute) {
			$var = codegen\codegen_member_var($attribute->getName())->setType($attribute->getWriteTypeHint());
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
			codegen\codegen_method('startDirtyTagging')
			->setReturnType('void')
			->setPrivate()
			->setBody(
				sprintf('$this->dirty = new Map([%s]);', $attribute_dirty_tags)
			)
		);
	}

	private function writeInsert(): void {
		$keyname = $this->harm->getPrimaryKeyName();

		$this->class->addMethod(
			codegen\codegen_method('insert')
			->setReturnType('void')
			->setPrivate()
			->setBody(
				"%s\n%s\n\t%s\n%s\n%s\n%s\n%s\n%s",
				'$dirty_for_write = $this->getDirtyForWrite();',
				sprintf('if ($this->%s != 0) {', $keyname),
				sprintf('$dirty_for_write->add(Pair{\'%s\', (string) $this->%s});', $keyname, $keyname),
				'}',
				sprintf(
					'$this->database->query(\'INSERT INTO %s (\'.implode(\', \', $dirty_for_write->keys()).\') VALUES ( \'.implode(\', \', $dirty_for_write->values()).\')\');',
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
			codegen\codegen_method('update')
			->setReturnType('void')
			->setPrivate()
			->setBody(
				"%s\n\t%s\n%s\n%s\n%s\n\t%s\n%s\n%s",
				'if (!$this->isModified()) {',
				'return;',
				'}',
				'$attribute_cast_list = Vector{};',
				'foreach ($this->getDirtyForWrite() as $field => $value) {',
				'$attribute_cast_list[] = $field.\' = \'.(string) $value;',
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
			$db_write_cast = $attribute->getDBWriteCast();
			$body .= 'if ($this->isDirty(\''.$name.'\')) {'."\n";
			$body .= "\t".'$attributes[\''.$name.'\'] = $this->'.$name.' === null ? \'null\' : '.$write_cast.'.\''.$db_write_cast.'\';'."\n";
			$body .= '}'."\n";
		}

		$this->class->addMethod(
			codegen\codegen_method('getDirtyForWrite')
			->setReturnType('Map<string, string>')
			->setPrivate()
			->setBody(
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
			codegen\codegen_method('getById')
			->addParameter('int $id')
			->setReturnType(sprintf('?%sInterface', $this->harm->getClassName()))
			->setBody(
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
			codegen\codegen_method('getObjectsBy')
			->addParameter('?string $condition = null')
			->addParameter('?string $order = null')
			->addParameter('?string $addendum = null')
			->setReturnType(sprintf('Vector<%sInterface>', $this->harm->getClassName()))
			->setBody(
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
			codegen\codegen_method('findObject')
			->addParameter('?string $condition = null')
			->setReturnType(sprintf('%sInterface', $this->harm->getClassName()))
			->setBody(
				"%s\n%s\n\t%s\n%s\n%s",
				'$iterator = $this->getObjectsBy($condition, null, \'LIMIT 1\');',
				'if ($iterator->count() === 0) {',
				'throw new \Usox\HaRm\Exception\ObjectNotFoundException();',
				'}',
				'return $iterator->items()->getIterator()->current();'
			)
		);
	}

	private function writeCount(): void {
		$this->class->addMethod(
			codegen\codegen_method('count')
			->addParameter('?string $condition = null')
			->setReturnType('int')
			->setBody(
				"%s\n%s",
				'$condition !== null ? $condition = sprintf(\'WHERE %s\', $condition) : $condition = \'\';',
				sprintf('return $this->database->count(sprintf(\'SELECT COUNT(%s) as count FROM %%s %%s\', $this->getTableName(), $condition));', $this->harm->getPrimaryKeyName()),
			)
		);
	}

	private function writeExists(): void {
		$this->class->addMethod(
			codegen\codegen_method('exists')
			->addParameter('?string $condition = null')
			->setReturnType('bool')
			->setBody(
				"%s\n%s",
				'$condition !== null ? $condition = sprintf(\'WHERE %s\', $condition) : $condition = \'\';',
				'return $this->database->exists(sprintf(\'SELECT 1 FROM %s %s\', $this->getTableName(), $condition));',
			)
		);
	}

	private function writeDelete(): void {
		$this->class->addMethod(
			codegen\codegen_method('delete')
			->setReturnType('void')
			->setBody(
				sprintf(
					'$this->database->query(\'DELETE FROM %s WHERE %s = \'.$this->getId());',
					$this->harm->getTableName(),
					$this->harm->getPrimaryKeyName()
				)
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
				codegen\codegen_method(sprintf('get%s', $accessor_name))
				->setReturnType($attribute->getReadCast())
				->setBody(
					sprintf('return $this->%s;', $attribute_name)
				)
			);
			$this->class->addMethod(
				codegen\codegen_method(sprintf('set%s', $accessor_name))
				->addParameter(sprintf('%s $value', $attribute->getWriteTypeHint()))
				->setReturnType('void')
				->setBody(
					"%s\n%s",
					sprintf('$this->tagDirty(\'%s\');', $attribute_name),
					sprintf('$this->%s = $value;', $attribute_name)
				)
			);
		}
	}

	private function writeSave(): void {
		$this->class->addMethod(
			codegen\codegen_method('save')
			->setReturnType('void')
			->setBody(
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
			codegen\codegen_method('isNew')
			->setReturnType('bool')
			->setBody('return $this->data_loaded !== true;')
		);
	}

	private function writeGetTableName(): void {
		$this->class->addMethod(
			codegen\codegen_method('getTableName')
			->setReturnType('string')
			->setBody(
				sprintf('return \'%s\';', $this->harm->getTableName())
			)
		);
	}

	private function writeConstructor(): void {
		$this->class->addMethod(
			codegen\codegen_method('__construct')
			->addParameter('private DatabaseInterface $database')
			->setReturnType('void')
			->setBody('$this->startDirtyTagging();')
		);
	}

	private function writeLoadDataByDatabaseResult(): void {
		$keyname = $this->harm->getPrimaryKeyName();
		$body = '';

		foreach ($this->harm->getAttributes() as $attribute) {
			$body .= sprintf('$this->set%s((%s) $data[\'%s\']);'."\n", $attribute->getAccessorName(), $attribute->getWriteTypeHint(), $attribute->getName());
		}

		$this->class->addMethod(
			codegen\codegen_method('loadDataByDatabaseResult')
			->addParameter('array<string, ?string> $data')
			->setReturnType('void')
			->setBody(
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
			codegen\codegen_method('tagDirty')
			->addParameter('string $attribute')
			->setReturnType('void')
			->setPrivate()
			->setBody(
				"%s\n%s",
				'$this->dirty[$attribute] = true;',
				'$this->modified = true;'
			)
		);
		$this->class->addMethod(
			codegen\codegen_method('isDirty')
			->addParameter('string $attribute')
			->setReturnType('bool')
			->setPrivate()
			->setBody('return $this->dirty[$attribute] === true;')
		);
		$this->class->addMethod(
			codegen\codegen_method('isModified')
			->setReturnType('bool')
			->setPrivate()
			->setBody('return $this->modified;')
		);
	}
}
