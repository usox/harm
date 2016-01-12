<?hh // strict
namespace Usox\HaRm\Writer\Table;

use Usox\HaRm\Generator\HarmGenerator;

final class StartDirtyTaggingWriter {

	public function __construct(private HarmGenerator $pobj): void {
	}

	public function writeOut(): void {
		$tab = "\t";
		$eol = PHP_EOL;
		$keyname = $this->pobj->getKeyName();
		$attributes = $this->pobj->getAttributes();
		$attribute_dirty_tags = '';

		printf('%sprivate function startDirtyTagging(): void {%s', $tab, $eol);
		foreach ($attributes as $attribute) {
			$attribute_dirty_tags .= sprintf('\'%s\' => false,', $attribute->getName());
		}

		printf('%s$this->dirty = new Map([%s]);%s', $tab.$tab, $attribute_dirty_tags, PHP_EOL);
		
		printf('%s}%s', $tab, PHP_EOL);
	}
}
