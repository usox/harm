<?hh // strict
namespace Usox\HaRm\Writer\Table;

use Usox\HaRm\Generator\HarmGenerator;

final class ExistsWriter {

	public function __construct(private HarmGenerator $pobj): void {
	}

	public function writeOut(): void {
		$tab = "\t";
		$two_tabs = $tab.$tab;
		printf(
			'%spublic function exists(?string $condition = null): bool {%s',
			$tab,
			PHP_EOL
		);
		printf(
			'%s$condition !== null ? $condition = sprintf(\'WHERE %%s\', $condition) : $condition = \'\';%s',
			$two_tabs,
			PHP_EOL
		);
		printf(
			'%sreturn $this->database->exists(sprintf(%s',
			$two_tabs,
			PHP_EOL
		);

		printf(
			'%s\'SELECT EXISTS(SELECT 1 FROM %s %%s LIMIT 1\', $condition%s',
			$two_tabs.$tab,
			$this->pobj->getTableName(),
			PHP_EOL

		);

		printf('%s));%s', $two_tabs, PHP_EOL);
		
		printf('%s}%s', $tab, PHP_EOL);
	}
}
