<?hh // strict
namespace Usox\HaRm\Writer\Table;

use Usox\HaRm\Generator\HarmGenerator;

final class UpdateWriter {

	public function __construct(private HarmGenerator $pobj): void {
	}

	public function writeOut(): void {
		$tab = "\t";
		$two_tabs = $tab.$tab;
		printf(
			'%sprivate function update(): void {%s',
			$tab,
			PHP_EOL
		);

		printf('%sif (!$this->isModified()) {%s', $two_tabs, PHP_EOL);
		printf('%sreturn;%s', $two_tabs.$tab, PHP_EOL);
		printf('%s}%s', $two_tabs, PHP_EOL);

		printf('%s$attribute_cast_list = Vector{};%s', $two_tabs, PHP_EOL);
		printf('%sforeach ($this->getDirtyForWrite() as $field => $value) {%s', $two_tabs, PHP_EOL);
		printf('%s$attribute_cast_list[] = $field.\' = \'.(string) $value;%s', $two_tabs.$tab, PHP_EOL);
		printf('%s}%s', $two_tabs, PHP_EOL);

		printf(
			'%s$this->database->query(\'UPDATE %s SET \'.implode(\', \', $attribute_cast_list).\' WHERE %s = \'.$this->getId());%s',
			$two_tabs,
			$this->pobj->getTableName(),
			$this->pobj->getKeyName(),
			PHP_EOL
		);
		
		printf('%s}%s', $tab, PHP_EOL);
	}
}
