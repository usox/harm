<?hh // strict
namespace Usox\HaRm\Generator;

use namespace Facebook\HackCodegen as codegen;

final class DbAttribute {

	private ?string $accessor_name;

	public function __construct(private string $name, private string $type): void {
	}

	public function getAccessorName(): string {
		if ($this->accessor_name === null) {
			$this->accessor_name = \str_replace(
				'_',
				'',
				\ucwords($this->name, '_')
			);
		}
		return $this->accessor_name;	
	}

	public function setDefaultValue(codegen\CodegenProperty $member_var): void {
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
			case 'float':
			case 'numeric':
				return '(string) '.$attribute;
			case 'timestamp':
				return '$this->database->quote((string) \date(\DATE_ATOM, '.$attribute.'))';
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

	public function getReadCast(string $attribute): string {
		switch ($this->type) {
		case 'timestamp':
			return '\strtotime('.$attribute.')';
		default:
			return $attribute;
		}
	}

	public function getDBReadCast(): string {
		return $this->name;
	}

	public function getName(): string {
		return $this->name;
	}
}
