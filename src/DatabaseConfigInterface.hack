namespace Usox\HaRm;

interface DatabaseConfigInterface {

	public function getHost(): string;

	public function getPort(): int;

	public function getName(): string;

	public function getUser(): string;

	public function getPassword(): string;

	public function getDriver(): string;

	public function __toString(): string;
}
