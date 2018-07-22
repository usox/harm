<?hh // strict
namespace Usox\HaRm\Writer;

use namespace HH\Lib\Str;
use type Usox\HaRm\Generator\HarmGenerator;
use type Facebook\HackCodegen\HackCodegenFactory;
use type Facebook\HackCodegen\HackCodegenConfig;
use type Facebook\HackCodegen\CodegenFileType;
use type Facebook\HackCodegen\CodegenFile;
use type Facebook\HackCodegen\CodegenInterface;

final class InterfaceWriter {

	private CodegenFile $file;
	private CodegenInterface $class;

	private HackCodegenFactory $cg_factory;

	public function __construct(private HarmGenerator $harm): void {
		$this->cg_factory = new HackCodegenFactory(
			new HackCodegenConfig()
		);

		$this->file = $this->cg_factory
			->codegenFile(
				Str\format('%sInterface.hh', $this->harm->getClassName())
			)
			->setDoClobber(true)
			->setFileType(CodegenFileType::HACK_STRICT)
			->setNamespace(
				$this->harm->getNamespaceName()
			);

		$this->class = $this->cg_factory
			->codegenInterface(
				Str\format('%sInterface', $this->harm->getClassName())
			);
	}

	public function writeOut(): void {
		$classname = $this->harm->getClassName();

		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('delete')
				->setReturnType('void')
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('save')
				->setReturnType('void')
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('isNew')
				->setReturnType('bool')
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('loadDataByDatabaseResult')
				->addParameter('array<string, ?string> $result')
				->setReturnType('void')
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('getTableName')
				->setReturnType('string')
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('getId')
				->setReturnType('int')
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('count')
				->addParameter('?string $where = null')
				->setReturnType('int')
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('exists')
				->addParameter('?string $where = null')
				->setReturnType('bool')
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('quote')
				->addParameter('string $subject')
				->setReturnType('string')
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('findObject')
				->addParameter('?string $condition = null')
				->setReturnType(
					Str\format('%sInterface', $classname)
				)
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('getObjectsBy')
				->addParameter('?string $condition = null')
				->addParameter('?string $order = null')
				->addParameter('?string $addendum = null')
				->setReturnType(
					Str\format('Vector<%sInterface>', $classname)
				)
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('getById')
				->addParameter('int $id')
				->setReturnType(
					Str\format('?%sInterface', $classname)
				)
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('emptyTable')
				->setReturnType('void')
		);
		foreach ($this->harm->getAttributes() as $attribute) {
			$readcast = $attribute->getWriteTypeHint();
			$accessor_name = $attribute->getAccessorName();

			$this->class->addMethod(
				$this->cg_factory
					->codegenMethod(
						Str\format('get%s', $accessor_name)
					)
					->setReturnType($readcast)
			);
			$this->class->addMethod(
				$this->cg_factory
					->codegenMethod(
						Str\format('set%s', $accessor_name)
					)
					->addParameter(Str\format('%s $value', $readcast))
					->setReturnType('void')
			);
		}
		
		$this->file->addClass($this->class);
		echo $this->file->render();
	}
}
