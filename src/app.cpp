#include "easer/easer.h"

struct Test {
	BEGIN();
	FIELD(int, a) = 2;
	FIELD(bool, b) = true;
	FIELD(char, c) = 's';
	END;
};

int main() {
	Test test{};
	std::ofstream file("test.dat");

	easer::Serializer::serialize(test, file);
	
	return 0;
}
