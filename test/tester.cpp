#define BOOST_TEST_MODULE Tester

typedef unsigned int value_type;
#include <boost/test/included/unit_test.hpp>
#include <easer/easer.h>

struct TestRecord {
	BEGIN();
	FIELD(int, a);
	FIELD(bool, b);
	FIELD(char, c);
	END;
};

BOOST_AUTO_TEST_CASE(first_test) {
	TestRecord record{2, false, 'y'};
	TestRecord in_record{};
	{
		std::ofstream file("first_test.dat");
		easer::Serializer::serialize(record, file);
	}
	{
		std::ifstream file("first_test.dat");
		easer::Serializer::deserialize(in_record, file);
	}

	BOOST_TEST(record.a == in_record.a);
	BOOST_TEST(record.b == in_record.b);
	BOOST_TEST(record.c == in_record.c);
}

struct TestRecordDerivedNoSerializable : public TestRecord {
	int d;
	bool e;
	char f;
};

BOOST_AUTO_TEST_CASE(second_test) {
	TestRecordDerivedNoSerializable record{{3, true, 'n'}, 2, false, 'y'};
	TestRecordDerivedNoSerializable in_record{};
	{
		std::ofstream file("second_test.dat");
		easer::Serializer::serialize(record, file);
	}
	{
		std::ifstream file("second_test.dat");
		easer::Serializer::deserialize(in_record, file);
	}

	BOOST_TEST(record.a == in_record.a);
	BOOST_TEST(record.b == in_record.b);
	BOOST_TEST(record.c == in_record.c);
}

struct TestRecordDerivedSerializable : public TestRecord {
	BEGIN(TestRecord);
	FIELD(int, d);
	FIELD(bool, e);
	FIELD(char, f);
	END;
};

BOOST_AUTO_TEST_CASE(third_test) {
	TestRecordDerivedSerializable record{{3, true, 'n'}, 2, false, 'y'};
	TestRecordDerivedSerializable in_record{};
	{
		std::ofstream file("third_test.dat");
		easer::Serializer::serialize(record, file);
	}
	{
		std::ifstream file("third_test.dat");
		easer::Serializer::deserialize(in_record, file);
	}

	BOOST_TEST(record.a == in_record.a);
	BOOST_TEST(record.b == in_record.b);
	BOOST_TEST(record.c == in_record.c);
	BOOST_TEST(record.d == in_record.d);
	BOOST_TEST(record.e == in_record.e);
	BOOST_TEST(record.f == in_record.f);
}

struct TestRecordDerivedTwiceSerializable : public TestRecordDerivedSerializable {
	BEGIN(TestRecordDerivedSerializable);
	FIELD(int, g);
	FIELD(bool, h);
	FIELD(char, i);
	END;
};

BOOST_AUTO_TEST_CASE(fourth_test) {
	TestRecordDerivedTwiceSerializable record{{{9, true, '/'}, 3, true, 'n'}, 2, false, 'y'};
	TestRecordDerivedTwiceSerializable in_record{};
	{
		std::ofstream file("fourth_test.dat");
		easer::Serializer::serialize(record, file);
	}
	{
		std::ifstream file("fourth_test.dat");
		easer::Serializer::deserialize(in_record, file);
	}

	BOOST_TEST(record.a == in_record.a);
	BOOST_TEST(record.b == in_record.b);
	BOOST_TEST(record.c == in_record.c);
	BOOST_TEST(record.d == in_record.d);
	BOOST_TEST(record.e == in_record.e);
	BOOST_TEST(record.f == in_record.f);
	BOOST_TEST(record.g == in_record.g);
	BOOST_TEST(record.h == in_record.h);
	BOOST_TEST(record.i == in_record.i);
}

struct TestRecordDerivedTwiceSerializableMember : public TestRecordDerivedSerializable {
	BEGIN(TestRecordDerivedSerializable);
	FIELD(int, g);
	FIELD(TestRecord, test_record_member);
	FIELD(bool, h);
	FIELD(char, i);
	END;
};

BOOST_AUTO_TEST_CASE(fifth_test) {
	TestRecordDerivedTwiceSerializableMember record{{{9, true, '/'}, 3, true, 'n'}, 2, {27, false, 'x'}, false, 'y'};
	TestRecordDerivedTwiceSerializableMember in_record{};
	{
		std::ofstream file("fifth_test.dat");
		easer::Serializer::serialize(record, file);
	}
	{
		std::ifstream file("fifth_test.dat");
		easer::Serializer::deserialize(in_record, file);
	}

	BOOST_TEST(record.a == in_record.a);
	BOOST_TEST(record.b == in_record.b);
	BOOST_TEST(record.c == in_record.c);
	BOOST_TEST(record.d == in_record.d);
	BOOST_TEST(record.e == in_record.e);
	BOOST_TEST(record.f == in_record.f);
	BOOST_TEST(record.g == in_record.g);

	BOOST_TEST(record.test_record_member.a == in_record.test_record_member.a);
	BOOST_TEST(record.test_record_member.b == in_record.test_record_member.b);
	BOOST_TEST(record.test_record_member.c == in_record.test_record_member.c);

	BOOST_TEST(record.h == in_record.h);
	BOOST_TEST(record.i == in_record.i);
}