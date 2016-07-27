<?hh // strict
namespace Usox\HaRm\Generator;

use Facebook\HackCodegen as codegen;

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

	public function setDefaultValue(codegen\CodegenMemberVar $member_var): void {
		switch ($this->type) {
			case 'int':
			case 'int2':
			case 'timestamp':
				$member_var->setValue(0);
				return;
			case 'float':
				$member_var->setLiteralValue('0.00');
				return;
			case 'text':
			case 'numeric':
			default:
				$member_var->setValue('');
				return;
		}
	}

	public function getWriteCast(string $attribute): string {
		switch ($this->type) {
			case 'int':
			case 'int2':
			case 'timestamp':
			case 'float':
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

	public function getName(): string {
		return $this->name;
	}
}
