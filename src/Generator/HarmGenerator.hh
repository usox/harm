<?hh // strict
namespace Usox\HaRm\Generator;

use Usox\HaRm\Writer\TableWriter;
use Usox\HaRm\Writer\InterfaceWriter;
use Usox\HaRm\Exception;

final class HarmGenerator {

	public string $class_name = '';

	public Vector<DbAttribute> $attributes;

	public string $primary_key_name = '';

	public string $namespace_name = '';

	public string $table_name = '';

	public function __construct(private string $config_file_path): void {
		$this->attributes = Vector{};
	}

	public function parseConfigFile(): void {
		if (!\file_exists($this->config_file_path)) {
			throw new Exception\HarmFileNotFoundException(
				\sprintf('Could not find harm-file: %s', $this->config_file_path)
			);
		}

		$config_file = \file($this->config_file_path);
		$allowed_types = new Vector([
			'int', 'int2', 'float', 'text', 'timestamp', 'inet'
		]);

		foreach ($config_file as $line) {
			if (\strpos($line, '#') === 0) {
				continue;
			}
			if (\strpos($line, '=') === false) {
				continue;
			}
			list($key, $config_value) = \explode('=', $line, 2);
			$config_value = \trim($config_value);
			switch ($key) {
				case 'table':
					$this->table_name = $config_value;
					break;
				case 'class':
					$this->class_name = $config_value;
					break;
				case 'key':
					$this->primary_key_name = $config_value;
					break;
				case 'namespace':
					$this->namespace_name = $config_value;
					break;
				case 'attribute':
					if (\strpos($config_value, ':') === false) {
						throw new Exception\InvalidAttributeDeclarationException($config_value);
					}
					list($attribute_name, $attribute_type) = \explode(':', $config_value, 2);
					if ($allowed_types->linearSearch($attribute_type) === -1) {
						throw new Exception\InvalidAttributeTypeException($attribute_type);
					}
					$this->attributes->add(
						new DbAttribute(\trim($attribute_name), \trim($attribute_type))
					);
					break;
				default:
					throw new Exception\UnknownHarmFileKeyException(
						\sprintf('Unknown configuration key: %s', $key)
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

	public function getPrimaryKeyName(): string {
		return $this->primary_key_name;
	}

	public function getTableName(): string {
		return $this->table_name;
	}

	public function getAttributes(): Vector<DbAttribute> {
		return $this->attributes;
	}
}
