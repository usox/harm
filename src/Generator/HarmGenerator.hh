<?hh // strict
namespace Usox\HaRm\Generator;

use Usox\HaRm\Writer\TableWriter;
use Usox\HaRm\Writer\InterfaceWriter;
use Usox\HaRm\Exception\UnknownHarmFileKeyException;
use Usox\HaRm\Exception\HarmFileNotFoundException;

final class HarmGenerator {

	public string $class_name = '';

	public Vector<DbAttribute> $attributes;

	public string $key_name = '';

	public string $namespace_name = '';

	public string $table_name = '';

	public function __construct(private string $config_file_path): void {
		$this->attributes = Vector{};
	}

	public function parseConfigFile(): void {
		if (!file_exists($this->config_file_path)) {
			throw new HarmFileNotFoundException(
				sprintf('Could not find harm-file: %s', $this->config_file_path)
			);
		}

		$config_file = file($this->config_file_path);

		foreach ($config_file as $line) {
			if (strpos($line, '#') === 0) {
				continue;
			}
			if (strpos($line, '=') === false) {
				continue;
			}
			list($key, $value) = explode('=', $line, 2);
			switch ($key) {
				case 'table':
					$this->table_name = rtrim($value);
					break;
				case 'class':
					$this->class_name = rtrim($value);
					break;
				case 'key':
					$this->key_name = rtrim($value);
					break;
				case 'namespace':
					$this->namespace_name = rtrim($value);
					break;
				case 'attribute':
					list($name, $type) = explode(':', $value);
					$this->attributes[] = new DbAttribute(
						trim($name),
						trim($type)
					);
					break;
				default:
					throw new UnknownHarmFileKeyException(
						sprintf('Unknown configuration key: %s', $key)
					);
			}
		}
	}

	public function writeTable(): void {
		$writer = new TableWriter($this);
		$writer->writeOut();
	}

	public function writeInterface(): void {
		$writer = new InterfaceWriter($this);
		$writer->writeOut();
	}

	public function getNamespaceName(): string {
		return $this->namespace_name;
	}

	public function getClassName(): string {
		return $this->class_name;
	}

	public function getKeyName(): string {
		return $this->key_name;
	}

	public function getTableName(): string {
		return $this->table_name;
	}

	public function getAttributes(): Vector<DbAttribute> {
		return $this->attributes;
	}
}
