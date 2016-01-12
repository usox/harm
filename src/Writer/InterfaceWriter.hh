<?hh // strict
namespace Usox\HaRm\Writer;

use Usox\HaRm\Generator\HarmGenerator;

final class InterfaceWriter {

	private string $tab = "\t";
	private string $eol = PHP_EOL;

	public function __construct(private HarmGenerator $pobj): void {
	}

	public function writeOut(): void {
		$this->writeHeader();
		$this->writeDeclarations();
		$this->writeFooter();
	}

	private function writeDeclarations(): void {
		$classname = $this->pobj->getClassName();

		printf('%spublic function delete(): void;%s', $this->tab, $this->eol);
		printf('%spublic function save(): void;%s', $this->tab, $this->eol);
		printf('%spublic function loadDataByDatabaseResult(array<string, ?string>$result): void;%s', $this->tab, $this->eol);
		printf('%spublic function getTableName(): string;%s', $this->tab, $this->eol);
		printf('%spublic function getId(): int;%s', $this->tab, $this->eol);
		printf('%spublic function count(?string $where = null): int;%s', $this->tab, $this->eol);
		printf('%spublic function exists(?string $where = null): bool;%s', $this->tab, $this->eol);
		printf('%spublic function findObject(?string $condition = null): %sInterface;%s', $this->tab, $classname, $this->eol);
		printf('%spublic function getObjectsBy(?string $condition = null, ?string $order = null, ?string $addendum = null): Vector<%sInterface>;%s', $this->tab, $classname, $this->eol);
		printf('%spublic function getById(int $id): ?%sInterface;%s', $this->tab, $classname, $this->eol);
		foreach ($this->pobj->getAttributes() as $attribute) {
			$attribute->writeInterfaceDeclaration();
		}
	}

	private function writeHeader(): void {
		$tab = "\t";
		printf('<?hh // strict%s', PHP_EOL);
		printf('namespace %s;%s', $this->pobj->getNamespaceName(), PHP_EOL);
		printf(
			'interface %sInterface {%s',
			$this->pobj->getClassName(),
			PHP_EOL
		);
	}

	private function writeFooter(): void {
		printf('}');
	}
}
