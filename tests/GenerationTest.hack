namespace Usox\HaRm;

use function Facebook\FBExpect\expect;

class GenerationTest extends \Facebook\HackTest\HackTest {

	public function testSampleTableGeneration(): void {
		\ob_start();

		\system('bin/harmgen table tests/fixtures/sample.harm');

		$result = \ob_get_contents();

		\ob_end_clean();

		expect($result)
			->toBeSame(\file_get_contents('tests/fixtures/pre_generated_table'));
	}

	public function testSampleInterfaceGeneration(): void {
		\ob_start();

		\system('bin/harmgen interface tests/fixtures/sample.harm');

		$result = \ob_get_contents();

		\ob_end_clean();

		expect($result)
			->toBeSame(\file_get_contents('tests/fixtures/pre_generated_interface'));
	}
}
