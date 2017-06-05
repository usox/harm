<?hh // strict
namespace Usox\HaRm\Writer;

use Usox\HaRm\Generator\HarmGenerator;
use Facebook\HackCodegen\HackCodegenFactory;
use Facebook\HackCodegen\HackCodegenConfig;
use Facebook\HackCodegen\CodegenFileType;
use Facebook\HackCodegen\CodegenFile;
use Facebook\HackCodegen\CodegenInterface;

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
				sprintf('%sInterface.hh', $this->harm->getClassName())
			)
			->setDoClobber(true)
			->setFileType(CodegenFileType::HACK_STRICT)
			->setNamespace(
				$this->harm->getNamespaceName()
			);

		$this->class = $this->cg_factory
			->codegenInterface(
				sprintf('%sInterface', $this->harm->getClassName())
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
					sprintf('%sInterface', $classname)
				)
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('getObjectsBy')
				->addParameter('?string $condition = null')
				->addParameter('?string $order = null')
				->addParameter('?string $addendum = null')
				->setReturnType(
					sprintf('Vector<%sInterface>', $classname)
				)
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('getById')
				->addParameter('int $id')
				->setReturnType(
					sprintf('?%sInterface', $classname)
				)
		);
		$this->class->addMethod(
			$this->cg_factory
				->codegenMethod('empty')
				->setReturnType('void')
		);
		foreach ($this->harm->getAttributes() as $attribute) {
			$readcast = $attribute->getWriteTypeHint();
			$accessor_name = $attribute->getAccessorName();

			$this->class->addMethod(
				$this->cg_factory
					->codegenMethod(
						sprintf('get%s', $accessor_name)
					)
					->setReturnType($readcast)
			);
			$this->class->addMethod(
				$this->cg_factory
					->codegenMethod(
						sprintf('set%s', $accessor_name)
					)
					->addParameter(sprintf('%s $value', $readcast))
					->setReturnType('void')
			);
		}
		
		$this->file->addClass($this->class);
		echo $this->file->render();
	}
}
