<?hh // strict
namespace Usox\HaRm\Writer\Table;

use Usox\HaRm\Generator\HarmGenerator;

final class GetDirtForWriteWriter {

	public function __construct(private HarmGenerator $pobj): void {
	}

	public function writeOut(): void {
		$tab = "\t";
		$two_tabs = $tab.$tab;
		printf(
			'%sprivate function getDirtyForWrite(): Map<string, string> {%s',
			$tab,
			PHP_EOL
		);

		printf('%s$attributes = Map{};%s', $two_tabs, PHP_EOL);
		
		foreach ($this->pobj->getAttributes() as $attribute) {
			$name = $attribute->getName();
			print "\t\tif (\$this->isDirty('{$name}')) {\n";
			print "\t\t\t\$attributes['{$name}'] = \$this->{$name} === null ? 'null' : \"'\".{$attribute->getWriteCast("\$this->{$name}")}.\"'{$attribute->getDBWriteCast()}\";\n";
			print "\t\t}\n";
		}

		printf('%sreturn $attributes;%s', $two_tabs, PHP_EOL);

		printf('%s}%s', $tab, PHP_EOL);
	}
}
