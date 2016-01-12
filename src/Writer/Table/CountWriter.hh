<?hh // strict
namespace Usox\HaRm\Writer\Table;

use Usox\HaRm\Generator\HarmGenerator;

final class CountWriter {

	public function __construct(private HarmGenerator $pobj): void {
	}

	public function writeOut(): void {
		$tab = "\t";
		$two_tabs = $tab.$tab;
		printf(
			'%spublic function count(?string $condition = null): int {%s',
			$tab,
			PHP_EOL
		);
		printf(
			'%s$condition !== null ? $condition = sprintf(\'WHERE %%s\', $condition) : $condition = \'\';%s',
			$two_tabs,
			PHP_EOL
		);
		printf(
			'%sreturn $this->database->count(sprintf(%s',
			$two_tabs,
			PHP_EOL
		);

		printf(
			'%s\'SELECT COUNT(%s) as count FROM %s %%s\', $condition%s',
			$two_tabs.$tab,
			$this->pobj->getKeyName(),
			$this->pobj->getTableName(),
			PHP_EOL

		);

		printf('%s));%s', $two_tabs, PHP_EOL);
		
		printf('%s}%s', $tab, PHP_EOL);
	}
}
