<?hh // strict
namespace Usox\HaRm\Writer\Table;

use Usox\HaRm\Generator\HarmGenerator;

final class InsertWriter {

	public function __construct(private HarmGenerator $pobj): void {
	}

	public function writeOut(): void {
		$tab = "\t";
		$two_tabs = $tab.$tab;
		printf(
			'%spublic function insert(): void {%s',
			$tab,
			PHP_EOL
		);
		
		printf('%s$dirty_for_write = $this->getDirtyForWrite();%s', $two_tabs, PHP_EOL);
		printf('%sif ($this->%s != 0) {%s', $two_tabs, $this->pobj->getKeyName(), PHP_EOL);
		printf(
			'%s$dirty_for_write->add(Pair{\'%s\', (string) $this->%s});%s',
			$two_tabs.$tab, $this->pobj->getKeyName(), $this->pobj->getKeyName(), PHP_EOL
		);
		printf('%s}%s', $two_tabs, PHP_EOL);

		printf(
			'%s$this->database->query(\'INSERT INTO %s (\'.implode(\', \', $dirty_for_write->keys()).\') VALUES ( \'.implode(\', \', $dirty_for_write->values()).\')\');%s',
			$two_tabs,
			$this->pobj->getTableName(),
			PHP_EOL
		);

		printf('%s$this->id = $this->database->getLastInsertedId();%s', $two_tabs, PHP_EOL);

		printf('%s$this->data_loaded = true;%s', $two_tabs, PHP_EOL);
		printf('%s$this->startDirtyTagging();%s', $two_tabs, PHP_EOL);
		
		printf('%s}%s', $tab, PHP_EOL);
	}
}
