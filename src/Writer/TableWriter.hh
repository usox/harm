<?hh // strict
namespace Usox\HaRm\Writer;

use namespace HH\Lib\{Str, Vec};
use type Usox\HaRm\Generator\HarmGenerator;
use type Usox\HaRm\Generator\DbAttribute;
use type Facebook\HackCodegen\HackCodegenFactory;
use type Facebook\HackCodegen\HackCodegenConfig;
use type Facebook\HackCodegen\CodegenFile;
use type Facebook\HackCodegen\CodegenClass;

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
				Str\format('%s.hh', $this->harm->getClassName())
			)
			->setNamespace($this->harm->getNamespaceName())
			->useNamespace('Usox\HaDb')
			->useNamespace('HH\Lib\{C, Str, Vec}')
			->setDoClobber(true);

		$this->class = $this->cg_factory
			->codegenClass(
				$this->harm->getClassName()
			)
			->setIsFinal(true)
			->addInterface(
				$this->cg_factory
				->codegenImplementsInterface(
					Str\format('%sInterface', $this->harm->getClassName())
				)
			)
			->addConst('TABLE_NAME', $this->harm->getTableName())
			->addConst('SEQUENCE_NAME', $this->harm->getSequenceName())
			->addProperty(
				$this->cg_factory
					->codegenProperty($this->harm->getPrimaryKey()->getName())
					->setType('int')
					->setValue(0)
			)
			->addProperty(
				$this->cg_factory
					->codegenProperty('data_loaded')
					->setType('bool')
					->setValue(false)
			)
			->addProperty(
				$this->cg_factory
					->codegenProperty('modified')
					->setType('bool')
					->setValue(false)
			)
			->addProperty(
				$this->cg_factory
					->codegenProperty('dirty')
					->setType('dict<string, bool>')
			);
	}

	public function writeOut(): void {
		foreach ($this->harm->getAttributes() as $attribute) {
			$var = $this->cg_factory
				->codegenProperty($attribute->getName())
				->setType($attribute->getWriteTypeHint());

			$attribute->setDefaultValue($var);
			$this->class->addProperty($var);
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
		$this->writeEmpty();
		$this->writeGetDirtyForWrite();
		$this->writeUpdate();
		$this->writeInsert();
		$this->writeStartDirtyTaggingWriter();
		$this->writeFooter();
		$this->writePrimaryKeyGetter();

		$this->file->addClass($this->class);
		echo $this->file->render();
	}

	private function writeStartDirtyTaggingWriter(): void {
		$attributes = $this->harm->getAttributes();
		$attribute_dirty_tags = '';

		foreach ($attributes as $attribute) {
			$attribute_dirty_tags .= Str\format('\'%s\' => false,', $attribute->getName());
		}

		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('startDirtyTagging')
				->setReturnType('void')
				->setPrivate()
				->setBodyf(
					'$this->dirty = dict[%s];',
					$attribute_dirty_tags
				)
		);
	}

	private function writeInsert(): void {
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('insert')
				->setReturnType('void')
				->setPrivate()
				->setBodyf(
					"%s\n%s\n%s\n%s\n%s",
					'$dirty_for_write = $this->getDirtyForWrite();',
					Str\format(
						'$this->database->query(\'INSERT INTO %s (\'.Str\join(Vec\keys($dirty_for_write), \', \').\') VALUES (\'.Str\join($dirty_for_write, \', \').\')\');',
						$this->harm->getTableName(),
					),
					Str\format('$this->%s = $this->database->getLastInsertedId(static::SEQUENCE_NAME);', $this->harm->getPrimaryKey()->getName()),
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
					'if (!$this->modified) {',
					'return;',
					'}',
					'$attribute_cast_list = vec[];',
					'foreach ($this->getDirtyForWrite() as $field => $value) {',
					'$attribute_cast_list[] = $field.\' = \'.(string) $value;',
					'}',
					'if (C\count($attribute_cast_list) === 0) {',
					'return;',
					'}',
					Str\format(
						'$this->database->query(\'UPDATE %s SET \'.Str\join($attribute_cast_list, \', \').\' WHERE %s = \'.$this->getId());',
						$this->harm->getTableName(),
						$this->harm->getPrimaryKey()->getName(),
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
				->setReturnType('dict<string, string>')
				->setPrivate()
				->setBodyf(
					"%s\n%s\n%s",
					'$attributes = dict[];',
					$body,
					'return $attributes;'
				)
		);
	}

	private function writeGetById(): void {
		$attribute_list = Vec\concat(
			vec[$this->harm->getPrimaryKey()->getName()],
			Vec\map(
				$this->harm->getAttributes(),
				(DbAttribute $attribute) ==> $attribute->getDBReadCast()
			)
		);
		
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('getById')
				->addParameter('int $id')
				->setReturnType(Str\format('?%sInterface', $this->harm->getClassName()))
				->setBodyf(
					"%s\n%s\n%s\n%s\n\t%s\n%s\n%s\n%s",
					Str\format(
						'$query = \'SELECT %s FROM %s WHERE %s = \'.$id;',
						Str\join($attribute_list, ','),
						$this->harm->getTableName(),
						$this->harm->getPrimaryKey()->getName(),
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

	private function writeEmpty(): void {
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('emptyTable')
				->setReturnType('void')
				->setBodyf(
					"%s",
					'$this->database->emptyTable(static::TABLE_NAME);'
				)
		);
	}

	private function writeGetObjectsBy(): void {
		$attribute_list = Vec\concat(
			vec[$this->harm->getPrimaryKey()->getName()],
			Vec\map(
				$this->harm->getAttributes(),
				(DbAttribute $attribute) ==> $attribute->getDBReadCast()
			)
		);
		
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('getObjectsBy')
				->addParameter('?string $condition = null')
				->addParameter('?string $order = null')
				->addParameter('?string $addendum = null')
				->setReturnType(Str\format('vec<%sInterface>', $this->harm->getClassName()))
				->setBodyf(
					"%s\n%s\n%s\n%s\n%s\n%s\n%s\n\t%s\n\t%s\n\t%s\n%s\n%s",
					Str\join($attribute_list, ',') |> Str\format('$query = \'SELECT %s FROM %s\';', $$, $this->harm->getTableName()),
					'if ($condition !== null) { $query .= \' WHERE \'.$condition; }',
					'if ($order !== null) { $query .= \' ORDER BY \'.$order; }',
					'if ($addendum !== null) { $query .= \' \'.$addendum; }',
					'$query_result = $this->database->query($query);',
					'$results = vec[];',
					'while ($result = $this->database->getNextResult($query_result)) {',
					'$obj = new self($this->database);',
					'$obj->loadDataByDatabaseResult($result);',
					'$results[] = $obj;',
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
				->setReturnType(Str\format('%sInterface', $this->harm->getClassName()))
				->setBodyf(
					"%s\n%s\n\t%s\n%s\n%s",
					'$objects = $this->getObjectsBy($condition, null, \'LIMIT 1\');',
					'if (C\count($objects) === 0) {',
					'throw new \Usox\HaRm\Exception\ObjectNotFoundException();',
					'}',
					'return C\firstx($objects);'
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
					"%s\n  %s\n%s\n  %s\n%s\n%s",
					'if ($condition !== null) {',
					'$condition = Str\format(\'WHERE %s\', $condition);',
					'} else {',
					'$condition = \'\';',
					'}',
					Str\format('return $this->database->count(Str\format(\'SELECT COUNT(%s) as count FROM %%s %%s\', $this->getTableName(), $condition));', $this->harm->getPrimaryKey()->getName()),
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
					"%s\n  %s\n%s\n  %s\n%s\n%s",
					'if ($condition !== null) {',
					'$condition = Str\format(\'WHERE %s\', $condition);',
					'} else {',
					'$condition = \'\';',
					'}',
					'return $this->database->exists(Str\format(\'SELECT 1 FROM %s %s\', $this->getTableName(), $condition));',
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
					$this->harm->getPrimaryKey()->getName()
				)
		);
	}

	private function writeAttributeAccessors(): void {
		$attributes = $this->harm->getAttributes();
		foreach ($attributes as $attribute) {
			$accessor_name = $attribute->getAccessorName();
			$attribute_name = $attribute->getName();

			$this->class->addMethod(
				$this->cg_factory
					->codegenMethod(Str\format('get%s', $accessor_name))
					->setReturnType($attribute->getWriteTypeHint())
					->setBodyf(
						'return $this->%s;',
						$attribute_name
					)
			);
			$this->class->addMethod(
				$this->cg_factory
					->codegenMethod(Str\format('set%s', $accessor_name))
					->addParameter(Str\format('%s $value', $attribute->getWriteTypeHint()))
					->setReturnType('void')
					->setBodyf(
						"%s\n%s",
						Str\format('$this->tagDirty(\'%s\');', $attribute_name),
						Str\format('$this->%s = $value;', $attribute_name)
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
				->addParameter('private HaDb\DatabaseAdapterInterface $database')
				->setReturnType('void')
				->setBody('$this->startDirtyTagging();')
		);
	}

	private function writeLoadDataByDatabaseResult(): void {
		$keyname = $this->harm->getPrimaryKey()->getName();
		$body = '';

		foreach ($this->harm->getAttributes() as $attribute) {
			$body .= Str\format(
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
					Str\format('$this->%s = (int) $data[\'%s\'];', $keyname, $keyname),
					$body,
					'$this->startDirtyTagging();'
				)
		);
	}

	private function writePrimaryKeyGetter(): void {
		$attribute = $this->harm->getPrimaryKey();

		$this->class->addMethod($this->cg_factory
			->codegenMethod(Str\format('get%s', $attribute->getAccessorName()))
			->setReturnType($attribute->getWriteTypeHint())
			->setBodyf('return $this->%s;', $attribute->getName()));
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
	}
}
