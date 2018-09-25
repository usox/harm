<?hh // strict

namespace Usox\HaRm;

use namespace HH\Lib\Str;

abstract class AbstractDatabaseConfig implements DatabaseConfigInterface {

	public function __toString(): string {
		return Str\format(
			'%s:dbname=%s;host=%s;port=%d;user=%s;password=%s',
			$this->getDriver(),
			$this->getName(),
			$this->getHost(),
			$this->getPort(),
			$this->getUser(),
			$this->getPassword()
		);
	}
}
