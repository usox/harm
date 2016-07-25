<?hh // strict
namespace Usox\HaRm\Generator;

final class DbAttribute {

	private ?string $accessor_name;

	public function __construct(private string $name, private string $type): void {
	}

	public function getAccessorName(): string {
		if ($this->accessor_name === null) {
			$this->accessor_name = str_replace(
				'_',
				'',
				ucwords($this->name, '_')
			);
		}
		return $this->accessor_name;	
	}

	public function getDefaultValue(): string {
		switch ($this->type) {
			case 'int':
			case 'int2':
			case 'timestamp':
				return '0';
			case 'float':
				return '0.00';
			case 'text':
			case 'numeric':
			default:
				return '\'\'';
		}
	}

	public function getWriteCast(string $attribute): string {
		switch ($this->type) {
			case 'int':
			case 'int2':
			case 'timestamp':
			case 'float':
			case 'text':
			case 'numeric':
				return '(string) '.$attribute;
			default:
				return '$this->database->quote('.$attribute.')';
		}
	}

	public function getWriteTypeHint(): string {
		switch ($this->type) {
			case 'int':
			case 'int2':
			case 'timestamp':
				return 'int';
			case 'float':
				return 'float';
			default:
				return 'string';
		}
	}

	public function getReadCast(): string {
		switch ($this->type) {
		case 'int':
		case 'int2':
		case 'timestamp':
			return 'int';
		case 'float':
			return 'float';
		case 'text':
		case 'numeric':
		default:
			return 'string';
		}
	}

	public function getDBWriteCast(): string {
		switch ($this->type) {
		case 'timestamp':
			return '::int4::abstime::timestamp';
		default:
			return '';
		}
	}

	public function getDBReadCast(): string {
		switch ($this->type) {
		case 'timestamp':
			return 'EXTRACT( EPOCH FROM '.$this->name.') AS '.$this->name;
		default:
			return $this->name;
		}
	}


	public function writeAccessors(): void {
		$accessor_name = $this->getAccessorName();

		printf('
	public function get%s(): %s {
		return $this->%s;
	}
',
			$this->accessor_name,
			$this->getReadCast(),
			$this->name
		);

		printf('

	public function set%s(%s $value): void {
		$this->tagDirty(\'%s\');
		$this->%s = $value;
	}

',
			$this->accessor_name,
			$this->getWriteTypeHint(),
			$this->name,
			$this->name
		);
	}

	public function writeInterfaceDeclaration(): void {
		printf('
	public function get%s(): %s;
',
			$this->getAccessorName(),
			$this->getReadCast()
		);

		printf('
	public function set%s(%s $value): void;
',
			$this->getAccessorName(),
			$this->getReadCast()
		);
	}

	public function writeDeclaration(): void {
		printf(
			'%sprivate %s $%s = %s;%s',
			"\t",
			$this->getWriteTypeHint(),
			$this->name,
			$this->getDefaultValue(),
			PHP_EOL
		);
	}

	public function writeDataInitialization(): void {
		printf('%s$this->set%s((%s) $data[\'%s\']);%s',
			"\t\t",
			$this->getAccessorName(),
			$this->getWriteTypeHint(),
			$this->name,
			PHP_EOL
	      );
	}

	public function getName(): string {
		return $this->name;
	}
}
