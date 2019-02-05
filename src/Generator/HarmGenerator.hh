<?hh // strict
namespace Usox\HaRm\Generator;

use namespace HH\Lib\{C, Str};
use type Usox\HaRm\Writer\TableWriter;
use type Usox\HaRm\Writer\InterfaceWriter;
use namespace Usox\HaRm\Exception;

final class HarmGenerator {

	public string $class_name = '';

	public vec<DbAttribute> $attributes = vec[];

	public ?DbAttribute $primary_key;

	public string $namespace_name = '';

	public string $table_name = '';

	private vec<string> $allowed_types = vec['int', 'int2', 'float', 'text', 'timestamp', 'inet', 'bool'];

	public function __construct(private string $config_file_path): void {
	}

	public function parseConfigFile(): void {
		if (!\file_exists($this->config_file_path)) {
			throw new Exception\HarmFileNotFoundException(
				Str\format('Could not find harm-file: %s', $this->config_file_path)
			);
		}

		$config_file = \file($this->config_file_path);

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
					$this->primary_key = new DbAttribute(Str\trim($config_value), 'int');
					break;
				case 'namespace':
					$this->namespace_name = $config_value;
					break;
				case 'attribute':
					if (\strpos($config_value, ':') === false) {
						throw new Exception\InvalidAttributeDeclarationException($config_value);
					}
					list($attribute_name, $attribute_type) = \explode(':', $config_value, 2);
					if (C\contains($this->allowed_types, $attribute_type) === false) {
						throw new Exception\InvalidAttributeTypeException($attribute_type);
					}
					$this->attributes[] = new DbAttribute(\trim($attribute_name), \trim($attribute_type));
					break;
				default:
					throw new Exception\UnknownHarmFileKeyException(
						Str\format('Unknown configuration key: %s', $key)
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

	public function getPrimaryKey(): DbAttribute {
		invariant($this->primary_key !== null, 'No primary key defined');
		return $this->primary_key;
	}

	public function getTableName(): string {
		return $this->table_name;
	}

	public function getSequenceName(): string {
		return Str\format(
			'%s_%s_seq',
			$this->getTableName(),
			$this->getPrimaryKey()->getName()
		);
	}

	public function getAttributes(): vec<DbAttribute> {
		return $this->attributes;
	}
}
