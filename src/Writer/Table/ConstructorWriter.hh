<?hh // strict
namespace Usox\HaRm\Writer\Table;

use Usox\HaRm\Generator\HarmGenerator;

final class ConstructorWriter {

	public function __construct(private HarmGenerator $pobj): void {
	}

	public function writeOut(): void {
		$tab = "\t";
		printf(
			'%spublic function __construct(private DatabaseInterface $database): void {%s',
			$tab,
			PHP_EOL
		);

		printf('%s$this->startDirtyTagging();%s', $tab.$tab, PHP_EOL);
		printf('%s}%s', $tab, PHP_EOL);
	}
}
