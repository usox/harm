<?hh // partial
namespace Usox\HaRm;

class GenerationTest extends \PHPUnit_Framework_TestCase {

	public function testSampleTableGeneration() {
		$this->expectOutputString(
			file_get_contents('tests/pre_generated_table')
		);
		
		system('hhvm bin/harmgen table tests/sample.harm');
	}

	public function testSampleInterfaceGeneration() {
		$this->expectOutputString(
			file_get_contents('tests/pre_generated_interface')
		);
		
		system('hhvm bin/harmgen interface tests/sample.harm');
	}
}  
