namespace Usox\HaRm\Generator;

use namespace Facebook\HackCodegen as codegen;
use namespace HH\Lib\Str;

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
				$member_var->setValue(
					0,
					codegen\HackBuilderValues::export()
				);
				return;
			case 'float':
				$lambda = function (codegen\IHackCodegenConfig $config, float $value): string {
					return \number_format($value, 1);
				};
				$member_var->setValue(
					0.0,
					codegen\HackBuilderValues::lambda($lambda)
				);
				return;
			case 'bool':
				$member_var->setValue(
					false,
					codegen\HackBuilderValues::export()
				);
				return;				
			case 'text':
			case 'numeric':
			default:
				$member_var->setValue(
					'',
					codegen\HackBuilderValues::export()
				);
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
			case 'bool':
				return Str\format('(bool) %s', $attribute);
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
			case 'bool':
				return 'bool';					
			default:
				return 'string';
		}
	}

	public function getReadCast(string $attribute): string {
		switch ($this->type) {
		case 'timestamp':
			return '\strtotime((string)'.$attribute.')';
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
