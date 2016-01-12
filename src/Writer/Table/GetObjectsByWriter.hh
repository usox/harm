<?hh // strict
namespace Usox\HaRm\Writer\Table;

use Usox\HaRm\Generator\HarmGenerator;

final class GetObjectsByWriter {

	public function __construct(private HarmGenerator $pobj): void {
	}

	public function writeOut(): void {
		$tab = "\t";
		$two_tabs = $tab.$tab;
		$three_tabs = $two_tabs.$tab;

		printf(
			'%spublic function getObjectsBy(
		?string $condition = null,
		?string $order = null,
		?string $addendum = null
	): Vector<%sInterface> {%s',
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
			'%s$query = \'SELECT %s FROM %s\';%s',
			$two_tabs,
			implode(',', $attributes_cast_list),
			$this->pobj->getTableName(),
			PHP_EOL
		);

		// WHERE
		printf('%sif ($condition !== null) {%s', $two_tabs, PHP_EOL);
		printf('%s$query .= \' WHERE \'.$condition;%s', $three_tabs, PHP_EOL);
		printf('%s}%s', $two_tabs, PHP_EOL);

		// ORDER
		printf('%sif ($order !== null) {%s', $two_tabs, PHP_EOL);
		printf('%s$query .= \' ORDER BY \'.$order;%s', $three_tabs, PHP_EOL);
		printf('%s}%s', $two_tabs, PHP_EOL);

		// ADDENDUM
		printf('%sif ($addendum !== null) {%s', $two_tabs, PHP_EOL);
		printf('%s$query .= \' \'.$addendum;%s', $three_tabs, PHP_EOL);
		printf('%s}%s', $two_tabs, PHP_EOL);

		printf('%s$query_result = $this->database->query($query);%s', $two_tabs, PHP_EOL);
		printf('%s$results = Vector{};%s', $two_tabs, PHP_EOL);

		printf('%swhile ($result = $this->database->getNextResult($query_result)) {%s', $two_tabs, PHP_EOL);
		printf('%s$obj = new self($this->database);%s', $three_tabs, PHP_EOL);
		printf('%s$obj->loadDataByDatabaseResult($result);%s', $three_tabs, PHP_EOL);
		printf('%s$results->add($obj);%s', $three_tabs, PHP_EOL);
		printf('%s}%s', $two_tabs, PHP_EOL);

		printf('%sreturn $results;%s', $two_tabs, PHP_EOL);
		
		printf('%s}%s', $tab, PHP_EOL);
	}
}
