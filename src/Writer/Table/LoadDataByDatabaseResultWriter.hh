<?hh // strict
namespace Usox\HaRm\Writer\Table;

use Usox\HaRm\Generator\HarmGenerator;

final class LoadDataByDatabaseResultWriter {

	public function __construct(private HarmGenerator $pobj): void {
	}

	public function writeOut(): void {
		$tab = "\t";
		$eol = PHP_EOL;
		$keyname = $this->pobj->getKeyName();

		printf('%spublic function loadDataByDatabaseResult(array<string, ?string> $data): void {%s', $tab, $eol);
		printf('%s$this->data_loaded = true;%s', $tab.$tab, $eol);
		printf('%s$this->modified = false;%s', $tab.$tab, $eol);
		
		printf('%s$this->%s = (int) $data[\'%s\'];%s', $tab.$tab, $keyname, $keyname, $eol);

		foreach ($this->pobj->getAttributes() as $attribute) {
			$attribute->writeDataInitialization();
		}

		printf('%s$this->startDirtyTagging();%s', $tab.$tab, PHP_EOL);
		
		printf('%s}%s', $tab, PHP_EOL);
	}
}
