<?hh // strict
namespace Usox\HaRm\Writer\Table;

use Usox\HaRm\Generator\HarmGenerator;

final class FindObjectWriter {

	public function __construct(private HarmGenerator $pobj): void {
	}

	public function writeOut(): void {
		$tab = "\t";
		$two_tabs = $tab.$tab;
		printf(
			'%spublic function findObject(?string $condition = null): %sInterface {%s',
			$tab,
			$this->pobj->getClassName(),
			PHP_EOL
		);
		printf('%s$iterator = $this->getObjectsBy($condition, null, \'LIMIT 1\');%s', $two_tabs, PHP_EOL);
		printf('%sif ($iterator->count() === 0) {%s', $two_tabs, PHP_EOL);
		printf('%sthrow new \Usox\HaRm\Exception\ObjectNotFoundException();%s', $two_tabs.$tab, PHP_EOL);
		printf('%s}%s', $two_tabs, PHP_EOL);
		printf('%sreturn $iterator->items()->getIterator()->current();%s', $two_tabs, PHP_EOL);
		
		printf('%s}%s', $tab, PHP_EOL);
	}
}
