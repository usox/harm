<?hh // strict
namespace Usox\HaRm\Writer;

use Usox\HaRm\Generator\HarmGenerator;
use Facebook\HackCodegen as codegen;

final class InterfaceWriter {

	private codegen\CodegenFile $file;
	private codegen\CodegenInterface $class;

	public function __construct(private HarmGenerator $harm): void {
		$this->file = codegen\codegen_file(
				sprintf('%sInterface.hh', $this->harm->getClassName())
			)
			->setIsStrict(true)
			->setNamespace($this->harm->getNamespaceName());

		$this->class = codegen\codegen_interface(
			sprintf('%sInterface', $this->harm->getClassName())
		);
	}

	public function writeOut(): void {
		$classname = $this->harm->getClassName();

		$this->class->addMethod(
			codegen\codegen_method('delete')->setReturnType('void')
		);
		$this->class->addMethod(
			codegen\codegen_method('save')->setReturnType('void')
		);
		$this->class->addMethod(
			codegen\codegen_method('loadDataByDatabaseResult')
			->addParameter('array<string, ?string> $result')
			->setReturnType('void')
		);
		$this->class->addMethod(
			codegen\codegen_method('getTableName')->setReturnType('string')
		);
		$this->class->addMethod(
			codegen\codegen_method('getId')->setReturnType('int')
		);
		$this->class->addMethod(
			codegen\codegen_method('count')
			->addParameter('?string $where = null')
			->setReturnType('int')
		);
		$this->class->addMethod(
			codegen\codegen_method('exists')
			->addParameter('?string $where = null')
			->setReturnType('bool')
		);
		$this->class->addMethod(
			codegen\codegen_method('findObject')
			->addParameter('?string $condition = null')
			->setReturnType(
				sprintf('%sInterface', $classname)
			)
		);
		$this->class->addMethod(
			codegen\codegen_method('getObjectsBy')
			->addParameter('?string $condition = null')
			->addParameter('?string $order = null')
			->addParameter('?string $addendum = null')
			->setReturnType(
				sprintf('Vector<%sInterface>', $classname)
			)
		);
		$this->class->addMethod(
			codegen\codegen_method('getById')
			->addParameter('int $id')
			->setReturnType(
				sprintf('?%sInterface', $classname)
			)
		);
		foreach ($this->harm->getAttributes() as $attribute) {
			$readcast = $attribute->getReadCast();
			$accessor_name = $attribute->getAccessorName();

			$this->class->addMethod(
				codegen\codegen_method(sprintf('get%s', $accessor_name))
				->setReturnType($readcast)
			);
			$this->class->addMethod(
				codegen\codegen_method(sprintf('set%s', $accessor_name))
				->addParameter(sprintf('%s $value', $readcast))
				->setReturnType('void')
			);
		}
		
		$this->file->addClass($this->class);
		echo $this->file->render();
	}
}
