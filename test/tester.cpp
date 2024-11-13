#define BOOST_TEST_MODULE Tester

#include <boost/test/included/unit_test.hpp>
#include <easer/easer.h>
#include <iostream>
#include <cstdio>
#include <vector>

constexpr char test_file_names[][128] = {"first_test.dat", "second_test.dat", "third_test.dat", "fourth_test.dat", "fifth_test.dat", "sixth_test.dat", "seventh_test.dat", "eight_test.dat", "ninth_test.dat", "", "eleventh_test.dat"};

struct TestRecord {
	EASER_BEGIN();
	EASER_FIELD(int, a, "test-channel");
	EASER_FIELD(bool, b, "test-channel");
	EASER_FIELD(char, c);
	EASER_END();
};

BOOST_AUTO_TEST_CASE(first_test) {
	TestRecord record{2, false, 'y'};
	TestRecord in_record{};
	{
		std::ofstream file(test_file_names[0]);
		easer::WriteStream<> stream(file);
		stream << record;
	}
	{
		std::ifstream file(test_file_names[0]);
		easer::ReadStream<> stream(file);
		stream >> in_record;
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
		std::ofstream file(test_file_names[1]);
		easer::WriteStream<> stream(file);
		stream << record;
	}
	{
		std::ifstream file(test_file_names[1]);
		easer::ReadStream<> stream(file);
		stream >> in_record;
	}

	BOOST_TEST(record.a == in_record.a);
	BOOST_TEST(record.b == in_record.b);
	BOOST_TEST(record.c == in_record.c);
}

struct TestRecordDerivedSerializable : public TestRecord {
	EASER_BEGIN(TestRecord);
	EASER_FIELD(int, d);
	EASER_FIELD(bool, e);
	EASER_FIELD(char, f);
	EASER_END();
};

BOOST_AUTO_TEST_CASE(third_test) {
	TestRecordDerivedSerializable record{{3, true, 'n'}, 2, false, 'y'};
	TestRecordDerivedSerializable in_record{};
	{
		std::ofstream file(test_file_names[2]);
		easer::WriteStream<> stream(file);
		stream << record;
	}
	{
		std::ifstream file(test_file_names[2]);
		easer::ReadStream<> stream(file);
		stream >> in_record;
	}

	BOOST_TEST(record.a == in_record.a);
	BOOST_TEST(record.b == in_record.b);
	BOOST_TEST(record.c == in_record.c);
	BOOST_TEST(record.d == in_record.d);
	BOOST_TEST(record.e == in_record.e);
	BOOST_TEST(record.f == in_record.f);
}

struct TestRecordDerivedTwiceSerializable : public TestRecordDerivedSerializable {
	EASER_BEGIN(TestRecordDerivedSerializable);
	EASER_FIELD(int, g);
	EASER_FIELD(bool, h);
	EASER_FIELD(char, i);
	EASER_END();
};

BOOST_AUTO_TEST_CASE(fourth_test) {
	TestRecordDerivedTwiceSerializable record{{{9, true, '/'}, 3, true, 'n'}, 2, false, 'y'};
	TestRecordDerivedTwiceSerializable in_record{};
	{
		std::ofstream file(test_file_names[3]);
		easer::WriteStream<> stream(file);
		stream << record;
	}
	{
		std::ifstream file(test_file_names[3]);
		easer::ReadStream<> stream(file);
		stream >> in_record;
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
	EASER_BEGIN(TestRecordDerivedSerializable);
	EASER_FIELD(int, g);
	EASER_FIELD(TestRecord, test_record_member);
	EASER_FIELD(bool, h);
	EASER_FIELD(char, i);
	EASER_END();
};

BOOST_AUTO_TEST_CASE(fifth_test) {
	TestRecordDerivedTwiceSerializableMember record{{{9, true, '/'}, 3, true, 'n'}, 2, {27, false, 'x'}, false, 'y'};
	TestRecordDerivedTwiceSerializableMember in_record{};
	{
		std::ofstream file(test_file_names[4]);
		easer::WriteStream<> stream(file);
		stream << record;
	}
	{
		std::ifstream file(test_file_names[4]);
		easer::ReadStream<> stream(file);
		stream >> in_record;
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

struct Vec2 {
	int x;
	int y;
};

template<EASER_REGISTRY_PARAMS>
EASER_REGISTER(Vec2, "", x, y);

template<EASER_REGISTRY_PARAMS>
EASER_REGISTER_NONE(Vec2, "test-channel");

BOOST_AUTO_TEST_CASE(sixth_test) {
	Vec2 record{12, 97};
	Vec2 in_record{};
	{
		std::ofstream file(test_file_names[5]);
		easer::WriteStream<> stream(file);
		stream << record;
	}
	{
		std::ifstream file(test_file_names[5]);
		easer::ReadStream<> stream(file);
		stream >> in_record;
	}

	BOOST_TEST(record.x == in_record.x);
	BOOST_TEST(record.y == in_record.y);
}

struct TestRecordVec2 {
	EASER_BEGIN();
	EASER_FIELD(Vec2, va, "test-channel");
	EASER_FIELD(Vec2, vb);
	EASER_END();
};

BOOST_AUTO_TEST_CASE(seventh_test) {
	TestRecordVec2 record{{ 12, 97 }, {48, 7}};
	TestRecordVec2 in_record{};
	{
		std::ofstream file(test_file_names[6]);
		easer::WriteStream<> stream(file);
		stream << record;
	}
	{
		std::ifstream file(test_file_names[6]);
		easer::ReadStream<> stream(file);
		stream >> in_record;
	}

	BOOST_TEST(record.va.x == in_record.va.x);
	BOOST_TEST(record.va.y == in_record.va.y);
	BOOST_TEST(record.vb.x == in_record.vb.x);
	BOOST_TEST(record.vb.y == in_record.vb.y);
}

struct TestRecordVec2RecordDerived : public TestRecordVec2, public TestRecord{
	EASER_BEGIN(TestRecordVec2, TestRecord);
	EASER_FIELD(int, vc);	
	EASER_FIELD(bool, vd);	
	EASER_FIELD(char, ve);	
	EASER_END();
};

BOOST_AUTO_TEST_CASE(eight_test) {
	TestRecordVec2RecordDerived record{{{ 12, 97 }, {48, 7}}, {89, true, 'q'}, 11, false, '0'};
	TestRecordVec2RecordDerived in_record{};
	{
		std::ofstream file(test_file_names[7]);
		easer::WriteStream<> stream(file);
		stream << record;
	}
	{
		std::ifstream file(test_file_names[7]);
		easer::ReadStream<> stream(file);
		stream >> in_record;
	}

	BOOST_TEST(record.va.x == in_record.va.x);
	BOOST_TEST(record.va.y == in_record.va.y);
	BOOST_TEST(record.vb.x == in_record.vb.x);
	BOOST_TEST(record.vb.y == in_record.vb.y);

	BOOST_TEST(record.a == in_record.a);
	BOOST_TEST(record.b == in_record.b);
	BOOST_TEST(record.c == in_record.c);

	BOOST_TEST(record.vc == in_record.vc);
	BOOST_TEST(record.vd == in_record.vd);
	BOOST_TEST(record.ve == in_record.ve);
}

struct TestRecordVec2RecordDerivedVec2 : public TestRecordVec2, public Vec2 {
	EASER_BEGIN(TestRecordVec2, Vec2);
	EASER_FIELD(TestRecord, test_record, "test-channel");
	EASER_END();
};

BOOST_AUTO_TEST_CASE(ninth_test) {
	TestRecordVec2RecordDerivedVec2 record{{{ 12, 97 }, {48, 7}}, {56, 79}, 11, true, '0'};
	TestRecordVec2RecordDerivedVec2 in_record{};
	{
		std::ofstream file(test_file_names[8]);
		easer::WriteStream<> stream(file);
		stream << record;
	}
	{
		std::ifstream file(test_file_names[8]);
		easer::ReadStream<> stream(file);
		stream >> in_record;
	}

	BOOST_TEST(record.va.x == in_record.va.x);
	BOOST_TEST(record.va.y == in_record.va.y);
	BOOST_TEST(record.vb.x == in_record.vb.x);
	BOOST_TEST(record.vb.y == in_record.vb.y);

	BOOST_TEST(record.x == in_record.x);
	BOOST_TEST(record.y == in_record.y);

	BOOST_TEST(record.test_record.a == in_record.test_record.a);
	BOOST_TEST(record.test_record.b == in_record.test_record.b);
	BOOST_TEST(record.test_record.c == in_record.test_record.c);
}

struct TestStream : public easer::Stream<TestStream> {
	TestStream(std::uint8_t* data, std::uint32_t size) : Stream<TestStream>(data, size) {
	}

	template<typename T> 
	static consteval std::uint32_t get_alignof() {
		return 2;
	}

	static consteval std::string_view get_channel() {
		return "test-channel";
	}
};

template<>
consteval std::uint32_t TestStream::get_alignof<char>() {
	return 4;
}

BOOST_AUTO_TEST_CASE(tenth_test) {
	TestRecordVec2RecordDerivedVec2 record{{{ 12, 97 }, {48, 7}}, {56, 79}, 11, true, '0'};
	TestRecordVec2RecordDerivedVec2 in_record{};
	{
		std::uint8_t buffer[512];
		easer::WriteStream<TestStream> wstream(buffer, sizeof(buffer));
		wstream << record;
		easer::ReadStream<TestStream> rstream(buffer, sizeof(buffer));
		rstream >> in_record;
	}

	BOOST_TEST(record.va.x != in_record.va.x);
	BOOST_TEST(record.va.y != in_record.va.y);
	BOOST_TEST(record.vb.x != in_record.vb.x);
	BOOST_TEST(record.vb.y != in_record.vb.y);

	BOOST_TEST(record.x != in_record.x);
	BOOST_TEST(record.y != in_record.y);

	BOOST_TEST(record.test_record.a == in_record.test_record.a);
	BOOST_TEST(record.test_record.b == in_record.test_record.b);
	BOOST_TEST(record.test_record.c != in_record.test_record.c);
}

template<typename T, EASER_REGISTRY_PARAMS>
EASER_REGISTER_PROC(std::vector<T>, "test-channel", v, stream, 
		{
			stream << v.size();
			for (auto& data : v) {
				stream << data;
			}
		},
		{
			std::size_t size;
			stream >> size;
			v.resize(size);
			for (auto& data : v) {
				stream >> data;
			}
		},
		{
			return v.size();
		});

template<typename T, EASER_REGISTRY_PARAMS>
EASER_REGISTER_PROC(std::vector<T>, "", v, stream, 
		{
		},
		{
		},
		{
			return v.size();
		});

BOOST_AUTO_TEST_CASE(eleventh_test) {
	std::vector<int> vec = {2, 4, 5, 10, 24};
	std::vector<int> in_vec;
	std::uint8_t buffer[512];
	
	{

		easer::WriteStream<TestStream> stream(buffer, sizeof(buffer));
		stream << vec;
	}
	{
		easer::ReadStream<TestStream> stream(buffer, sizeof(buffer));
		stream >> in_vec;
	}

	BOOST_TEST(vec.size() == in_vec.size());
	for (int i = 0; i < vec.size(); i++) {
		BOOST_TEST(vec[i] == in_vec[i]);
	}
}

struct MyStruct {
	EASER_BEGIN();
	EASER_SPEC_FIELD(static, int, i);
	EASER_END();
};

struct Config {
	Config() {
		std::cout << "Tester initialized\n";
	}

	~Config() {
		for (auto fname : test_file_names) {
			std::remove(fname);
		}
		std::cout << "Tester shutdown\n";
	}
};

BOOST_GLOBAL_FIXTURE(Config);
