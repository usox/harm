<?hh // strict
namespace Usox\HaRm\Writer\Table;

use Usox\HaRm\Generator\HarmGenerator;

final class GetByIdWriter {

	public function __construct(private HarmGenerator $pobj): void {
	}

	public function writeOut(): void {
		$tab = "\t";
		$two_tabs = $tab.$tab;
		printf(
			'%spublic function getById(int $id): ?%sInterface {%s',
			$tab,
			$this->pobj->getClassName(),
			PHP_EOL
		);
		
		$attributes_cast_list = Vector{};
		$attributes_cast_list[] = $this->pobj->getKeyName();
		foreach ($this->pobj->getAttributes() as $attribute) {
			$attributes_cast_list[] = $attribute->getDBReadCast();
		}
		printf(
			'%s$query = \'SELECT %s FROM %s WHERE %s = \'.$id;%s',
			$two_tabs,
			implode(',', $attributes_cast_list),
			$this->pobj->getTableName(),
			$this->pobj->getKeyName(),
			PHP_EOL
		);

		printf('%s$object = new self($this->database);%s', $two_tabs, PHP_EOL);
		printf('%s$data = $this->database->getNextResult($this->database->query($query));%s', $two_tabs, PHP_EOL);
		printf('%sif ($data === null) {%s', $two_tabs, PHP_EOL);
		printf('%sreturn null;%s', $two_tabs.$tab, PHP_EOL);
		printf('%s}%s', $two_tabs, PHP_EOL);

		printf('%s$object->loadDataByDatabaseResult($data);%s', $two_tabs, PHP_EOL);
		printf('%sreturn $object;%s', $two_tabs, PHP_EOL);
		
		printf('%s}%s', $tab, PHP_EOL);
	}
}
