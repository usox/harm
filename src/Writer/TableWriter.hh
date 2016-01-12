<?hh // strict
namespace Usox\HaRm\Writer;

use Usox\HaRm\Generator\HarmGenerator;
use Usox\HaRm\Generator\DbAttribute;

final class TableWriter {

	private string $tab = "\t";
	private string $eol = PHP_EOL;

	public function __construct(private HarmGenerator $pobj): void {
	}

	public function writeOut(): void {
		$this->writeHeader();
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
	}

	private function writeStartDirtyTaggingWriter(): void {
		$writer = new Table\StartDirtyTaggingWriter($this->pobj);
		$writer->writeOut();
	}

	private function writeInsert(): void {
		$writer = new Table\InsertWriter($this->pobj);
		$writer->writeOut();
	}

	private function writeUpdate(): void {
		$writer = new Table\UpdateWriter($this->pobj);
		$writer->writeOut();
	}

	private function writeGetDirtyForWrite(): void {
		$writer = new Table\GetDirtForWriteWriter($this->pobj);
		$writer->writeOut();
	}

	private function writeGetById(): void {
		$writer = new Table\GetByIdWriter($this->pobj);
		$writer->writeOut();
	}

	private function writeGetObjectsBy(): void {
		$writer = new Table\GetObjectsByWriter($this->pobj);
		$writer->writeOut();
	}

	private function writeFindObject(): void {
		$writer = new Table\FindObjectWriter($this->pobj);
		$writer->writeOut();
	}

	private function writeCount(): void {
		$writer = new Table\CountWriter($this->pobj);
		$writer->writeOut();
	}

	private function writeExists(): void {
		$writer = new Table\ExistsWriter($this->pobj);
		$writer->writeOut();
	}

	private function writeDelete(): void {
		printf('%spublic function delete(): void {%s', $this->tab, $this->eol);
		printf(
			'%s$this->database->query(\'DELETE FROM %s WHERE %s = \'.$this->getId());%s',
			$this->tab.$this->tab,
			$this->pobj->getTableName(),
			$this->pobj->getKeyName(),
			$this->eol
		);
		printf('%s}%s', $this->tab, $this->eol);
	}

	private function writeAttributeAccessors(): void {
		$key_attribute = new DbAttribute($this->pobj->getKeyName(), 'int');
		$key_attribute->writeAccessors();
		foreach ($this->pobj->getAttributes() as $attribute) {
			$attribute->writeAccessors();
		}
	}

	private function writeSave(): void {
		$three_tabs = str_repeat($this->tab, 3);
		$two_tabs = str_repeat($this->tab, 2);

		printf('%spublic function save(): void {%s', $this->tab, $this->eol);
		printf('%sif ($this->isNew()) {%s', $two_tabs, $this->eol);
		printf('%s$this->insert();%s', $three_tabs, $this->eol);
		printf('%s} else {%s', $two_tabs, $this->eol);
		printf('%s$this->update();%s', $three_tabs, $this->eol);
		printf('%s}%s', $two_tabs, $this->eol);
		printf('%s}%s', $this->tab, $this->eol);
	}

	private function writeIsNew(): void {
		$tab = "\t";
		$eol = PHP_EOL;
		printf('%spublic function isNew(): bool {%s', $tab, $eol);
		printf('%sreturn $this->data_loaded !== true;%s', $tab.$tab, $eol);
		printf('%s}%s', $tab, $eol);
	}

	private function writeGetTableName(): void {
		printf('%spublic function getTableName(): string {%s', $this->tab, $this->eol);
		printf('%sreturn \'%s\';%s', $this->tab.$this->tab, $this->pobj->getTableName(), $this->eol);
		printf('%s}%s', $this->tab, $this->eol);
	}

	private function writeConstructor(): void {
		$writer = new Table\ConstructorWriter($this->pobj);
		$writer->writeOut();
	}

	private function writeLoadDataByDatabaseResult(): void {
		$writer = new Table\LoadDataByDatabaseResultWriter($this->pobj);
		$writer->writeOut();
	}

	private function writeHeader(): void {
		$tab = "\t";
		printf('<?hh // strict%s', PHP_EOL);
		printf('namespace %s;%s', $this->pobj->getNamespaceName(), PHP_EOL);
		printf('use Usox\\HaDb\\DatabaseInterface;%s', PHP_EOL);
		printf(
			'final class %s implements %sInterface {%s',
			$this->pobj->getClassName(),
			$this->pobj->getClassName(),
			PHP_EOL
		);
		printf(
			'%sprivate int $%s = 0;%s',
			$tab,
			$this->pobj->getKeyName(),
			PHP_EOL
		);
		printf('%sprivate bool $data_loaded = false;%s', $tab, PHP_EOL);
		printf('%sprivate bool $modified = false;%s', $tab, PHP_EOL);
		printf('%sprivate Map<string, bool> $dirty;%s', $tab, PHP_EOL);

		foreach ($this->pobj->getAttributes() as $attribute) {
			$attribute->writeDeclaration();
		}
	}

	private function writeFooter(): void {
		$tab = "\t";
		printf('%sprivate function tagDirty(string $attribute): void {%s', $tab, PHP_EOL);
		printf('%s$this->dirty[$attribute] = true;%s', $tab.$tab, PHP_EOL);
		printf('%s$this->modified = true;%s', $tab.$tab, PHP_EOL);
		printf('%s}%s', $tab, PHP_EOL);
		printf('%sprivate function isDirty(string $attribute): bool {%s', $tab, PHP_EOL);
		printf('%sreturn $this->dirty[$attribute] === true;%s', $tab.$tab, PHP_EOL);
		printf('%s}%s', $tab, PHP_EOL);
		printf('%sprivate function isModified(): bool {%s', $tab, PHP_EOL);
		printf('%sreturn $this->modified;%s', $tab.$tab, PHP_EOL);
		printf('%s}%s', $tab, PHP_EOL);
		printf('}');
	}
}
