#ifndef EASER_EASER_H
#define EASER_EASER_H

#include <limits>
#include <fstream>
#include <functional>
#include <type_traits>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <boost/preprocessor.hpp>

#define ESR_EXPAND_MEMBER(instance, member) instance.member
#define ESR_EXPAND_MEMBER_TYPE(type, member) decltype((*(static_cast<type*>(nullptr))).member)
#define ESR_EXPAND_CHANNEL_LAMBDA(none, channel_name) [](){ return channel_name; }
#define ESR_EXPAND_CHANNEL_LAMBDA_TYPE(none, channel_name) decltype([](){ return channel_name; })

#define ESR_MAKE_CHANNELS(...) std::string_view{""} __VA_OPT__(, __VA_ARGS__)

#define ESR_PACK(...) __VA_ARGS__

#define ESR_EXPAND_MEMBER_HELPER(r, instance, member) \
	ESR_EXPAND_MEMBER(instance, member)

#define ESR_EXPAND_MEMBER_TYPE_HELPER(r, type, member) \
	ESR_EXPAND_MEMBER_TYPE(type, member)

#define ESR_EXPAND_CHANNEL_LAMBDA_HELPER(r, none, channel_name) \
	ESR_EXPAND_CHANNEL_LAMBDA(none, channel_name)

#define ESR_EXPAND_CHANNEL_LAMBDA_TYPE_HELPER(r, none, channel_name) \
	ESR_EXPAND_CHANNEL_LAMBDA_TYPE(none, channel_name)

#define ESR_EXPAND_MEMBER_HELPER(r, instance, member) \
	ESR_EXPAND_MEMBER(instance, member)

#define ESR_FIELD_NAME_VARIADIC_TO_FIELD_ACCESS_VARIADIC(instance, ...) \
	BOOST_PP_SEQ_ENUM( \
		BOOST_PP_SEQ_TRANSFORM(ESR_EXPAND_MEMBER_HELPER, instance, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	)

#define ESR_FIELD_NAME_VARIADIC_TO_FIELD_TYPE_VARIADIC(type, ...) \
	BOOST_PP_SEQ_ENUM( \
		BOOST_PP_SEQ_TRANSFORM(ESR_EXPAND_MEMBER_TYPE_HELPER, type, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	)

#define ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_VARIADIC(...) \
	BOOST_PP_SEQ_ENUM( \
		BOOST_PP_SEQ_TRANSFORM(ESR_EXPAND_CHANNEL_LAMBDA_HELPER, ~, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	)

#define ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(...) \
	BOOST_PP_SEQ_ENUM( \
		BOOST_PP_SEQ_TRANSFORM(ESR_EXPAND_CHANNEL_LAMBDA_TYPE_HELPER, ~, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	)

#define ESR_API(name) __esr__##name

#define ESR_REGISTRY_PARAMS typename ESR_API(TChannel)

#define ESR_REGISTER(type, channel_name, ...) \
		requires (::esr::Manager::is_in_set<ESR_API(TChannel), ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(channel_name)>()) \
	class ESR_API(Registry)<type, ESR_API(TChannel)> { \
	public: \
		static consteval bool is_valid() { \
			return true; \
		} \
		static consteval bool is_none() { \
			return false; \
		} \
	private: \
		template<typename TStream> \
		inline static void write(const type& v, ::esr::WriteStream<TStream>& stream) { \
			stream.template advance_write_ptr<type>(); \
			internal_write(v, stream, ESR_FIELD_NAME_VARIADIC_TO_FIELD_ACCESS_VARIADIC(v, __VA_ARGS__)); \
		} \
		template<typename TStream> \
		inline static void read(type& v, ::esr::ReadStream<TStream>& stream) { \
			stream.template advance_read_ptr<type>(); \
			internal_read(v, stream, ESR_FIELD_NAME_VARIADIC_TO_FIELD_ACCESS_VARIADIC(v, __VA_ARGS__)); \
		} \
		static consteval std::uint32_t get_sizeof() { \
			return internal_get_sizeof<ESR_FIELD_NAME_VARIADIC_TO_FIELD_TYPE_VARIADIC(type, __VA_ARGS__)>(); \
		} \
		template<typename TCurrent, typename TStream, typename... TArgs> \
		inline static void internal_write(const type& name, ::esr::WriteStream<TStream>& stream, TCurrent& write_ptr, TArgs&... args) { \
			stream.template advance_write_ptr<TCurrent>(); \
			::esr::Manager::write<TCurrent>(write_ptr, stream); \
			if constexpr(sizeof...(TArgs)) { \
				internal_write(name, stream, args...); \
			} \
		} \
		template<typename TCurrent, typename TStream, typename... TArgs> \
		inline static void internal_read(type& name, ::esr::ReadStream<TStream>& stream, TCurrent& write_ptr, TArgs&... args) { \
			::esr::Manager::read<TCurrent>(write_ptr, stream); \
			if constexpr(sizeof...(TArgs)) { \
				internal_read(name, stream, args...); \
			} \
		} \
		template<typename TCurrent, typename... TArgs> \
		static consteval std::uint32_t internal_get_sizeof() { \
			if constexpr(sizeof...(TArgs)) { \
				return sizeof(TCurrent) + internal_get_sizeof<TArgs...>(); \
			} \
			else { \
				return sizeof(TCurrent); \
			} \
		} \
		friend class ::esr::Manager; \
	}; \

#define ESR_REGISTER_NONE(type, channel_name) \
		requires (::esr::Manager::is_in_set<ESR_API(TChannel), ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(channel_name)>()) \
	class ESR_API(Registry)<type, ESR_API(TChannel)> { \
	public: \
		static consteval bool is_valid() { \
			return true; \
		} \
		static consteval bool is_none() { \
			return true; \
		} \
		static consteval std::uint32_t get_sizeof() { \
			return 0; \
		} \
		friend class ::esr::Manager; \
	}; \

#define ESR_REGISTER_PROC(type, channel_name, type_name, stream_name, write_func_body, read_func_body, get_sizeof_func_body) \
		requires (::esr::Manager::is_in_set<ESR_API(TChannel), ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(channel_name)>()) \
	class ESR_API(Registry)<type, ESR_API(TChannel)> { \
	public: \
		static consteval bool is_valid() { \
			return true; \
		} \
		static consteval bool is_none() { \
			return false; \
		} \
	private: \
		template<typename ESR_API(TStream)> \
		inline static void write(const type& type_name, ::esr::WriteStream<ESR_API(TStream)>& stream_name) { \
			stream_name.template advance_write_ptr<type>(); \
			write_func_body \
		} \
		template<typename ESR_API(TStream)> \
		inline static void read(type& type_name, ::esr::ReadStream<ESR_API(TStream)>& stream_name) { \
			stream_name.template advance_read_ptr<type>(); \
			read_func_body \
		} \
		static consteval std::uint32_t get_sizeof(type& type_name) { \
			get_sizeof_func_body \
		} \
		friend class ::esr::Manager; \
	}; \

#define ESR_BEGIN(...) \
	struct ESR_API(Inheritance) { \
		using Bases = std::tuple<__VA_ARGS__>; \
	}; \
	static consteval unsigned int ESR_API(begin)() { \
		const unsigned int begin_id = __COUNTER__; \
		static_assert(begin_id != std::numeric_limits<unsigned int>::max()); \
		return begin_id; \
	} \
	template<typename TChannel, typename TField> \
	static consteval bool ESR_API(is_field_valid)(TChannel, TField) { \
		return false; \
	} \
	template<typename T, typename V> \
	friend class ::esr::Manager::has_begin;

#define ESR_FIELD(field_type, field_name, ...) \
	template<typename TChannel> \
	static consteval bool ESR_API(is_field_valid)(TChannel channel, const ::esr::Manager::Field<__COUNTER__>) { \
		static_assert(::esr::Manager::has_register<field_type>() || ::esr::Manager::is_registered<field_type, TChannel>() || std::is_fundamental_v<field_type>, "T does not declare ESR_BEGIN() or ESR_END(), it is not globally serializable nor fundamental"); \
		return ::esr::Manager::is_in_set<TChannel, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ESR_MAKE_CHANNELS(__VA_ARGS__))>(); \
	} \
	template<typename TStream> \
	inline void ESR_API(write)(::esr::WriteStream<TStream>& stream, const ::esr::Manager::Field<__COUNTER__ - 1>) const { \
		::esr::Manager::write(field_name, stream); \
	} \
	template<typename TStream> \
	inline void ESR_API(read)(::esr::ReadStream<TStream>& stream, const ::esr::Manager::Field<__COUNTER__ - 2>) { \
		::esr::Manager::read(field_name, stream); \
	} \
	static consteval std::uint32_t ESR_API(get_sizeof)(const ::esr::Manager::Field<__COUNTER__ - 3>) { \
		return ::esr::Manager::get_sizeof<field_type>(); \
	} \
	field_type field_name

#define ESR_SPEC_FIELD(declaration_specifier, field_type, field_name, ...) \
	template<typename TChannel> \
	static consteval bool ESR_API(is_field_valid)(TChannel channel, const ::esr::Manager::Field<__COUNTER__>) { \
		static_assert(::esr::Manager::has_register<field_type>() || ::esr::Manager::is_registered<field_type, TChannel>() || std::is_fundamental_v<field_type>, "T does not declare ESR_BEGIN() or ESR_END(), it is not globally serializable nor fundamental"); \
		return ::esr::Manager::is_in_set<TChannel, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ESR_MAKE_CHANNELS(__VA_ARGS__))>(); \
	} \
	template<typename TStream> \
	inline void ESR_API(write)(::esr::WriteStream<TStream>& stream, const ::esr::Manager::Field<__COUNTER__ - 1>) const { \
		::esr::Manager::write(field_name, stream); \
	} \
	template<typename TStream> \
	inline void ESR_API(read)(::esr::ReadStream<TStream>& stream, const ::esr::Manager::Field<__COUNTER__ - 2>) { \
		::esr::Manager::read(field_name, stream); \
	} \
	static consteval std::uint32_t ESR_API(get_sizeof)(const ::esr::Manager::Field<__COUNTER__ - 3>) { \
		return ::esr::Manager::get_sizeof<field_type>(); \
	} \
	declaration_specifier field_type field_name

#define ESR_END() \
	static consteval unsigned int ESR_API(end()) { \
		return __COUNTER__; \
	} \
	friend class ::esr::Manager; \
	template<typename T, typename V> \
	friend class esr::Manager::has_end; \

template<typename T, typename TChannel>
struct ESR_API(Registry) {
	static consteval bool is_valid() {
		return false;
	}
};

namespace esr {
	class Manager;

	template<typename TStream>
	class Stream {
	public:
		template<typename T>
		inline static std::uint32_t get_alignof() {
			return alignof(T);
		}

		static consteval std::string_view get_channel() {
			return std::string_view{""};
		}

		inline const std::uint8_t* get_handle() const {
			return m_handle;
		}

		inline std::uint32_t get_size() const {
			return m_size;
		}

	protected:
		Stream(std::uint8_t* handle, std::uint32_t size) : 	
			m_handle(handle), 
			m_size(size) {

			static_assert(std::is_base_of_v<Stream<TStream>, TStream>);
		}

	private:
		std::uint8_t* m_handle;
		std::uint32_t m_size;
	};

	template<>
	class Stream<std::ios> {
	public:
		template<typename T>
		inline static std::uint32_t get_alignof() {
			return 1;
		}

		static consteval std::string_view get_channel() {
			return std::string_view{""};
		}

	protected:
		Stream() = default;
	};

	template <typename TStream>
	concept StreamDerivedFromStandard = (std::is_base_of_v<Stream<std::ios>, TStream> && !std::is_same_v<std::ios, TStream>);

	template<typename TStream = std::ios>
	class WriteStream : public Stream<TStream> {
	public:
		using StreamType = TStream;

		WriteStream(std::uint8_t* handle, std::uint32_t size) : 
			Stream<TStream>(handle, size), 
			m_write_ptr(handle) {
		}

		~WriteStream() {
		}

		template<typename T>
		inline WriteStream<TStream>& operator << (const T& value);

		inline const std::uint8_t* get_write_ptr() const {
			return m_write_ptr;
		}

		template<typename T>
		inline void advance_write_ptr();

	private:
		inline void write(const std::uint8_t* data, std::uint32_t size);

	private:
		std::uint8_t* m_write_ptr;

		friend class Manager;
	};

	template<>
	class WriteStream<std::ios> : public Stream<std::ios> {
	public:
		using StreamType = Stream<std::ios>;

		WriteStream(std::ostream& ostream) : 
			m_ostream(ostream),
			Stream<std::ios>()
		{
			
		}

		~WriteStream() {
		}

		template<typename T>
		inline WriteStream<std::ios>& operator << (const T& value);

		template<typename T>
		inline void advance_write_ptr();

	private:
		inline void write(const std::uint8_t* data, std::uint32_t size);

	private:
		std::ostream& m_ostream;

		friend class Manager;
	};

	template<StreamDerivedFromStandard TStream>
	class WriteStream<TStream> : public TStream {
	public:
		using StreamType = TStream;

		WriteStream(std::ostream& ostream, const TStream& stream = TStream()) : 
			m_ostream(ostream),
			TStream(stream)
		{
			
		}

		~WriteStream() {
		}

		template<typename T>
		inline WriteStream<TStream>& operator << (const T& value);

		template<typename T>
		inline void advance_write_ptr();

	private:
		inline void write(const std::uint8_t* data, std::uint32_t size);

	private:
		std::ostream& m_ostream;

		friend class Manager;
	};

	template<typename TStream = std::ios>
	class ReadStream : public Stream<TStream> {
	public:
		using StreamType = TStream;

		ReadStream(std::uint8_t* handle, std::uint32_t size) : 
			Stream<TStream>(handle, size),
			m_read_ptr(handle) {

		}

		template<typename T>
		inline ReadStream<TStream>& operator >> (T& value);

		inline const std::uint8_t* get_read_ptr() const {
			return m_read_ptr;
		}

		template<typename T>
		inline void advance_read_ptr();

	private:
		inline void read(std::uint8_t* data, std::uint32_t size);

	private:
		std::uint8_t* m_read_ptr;

		friend class Manager;
	};

	template<>
	class ReadStream<std::ios> : public Stream<std::ios> {
	public:
		using StreamType = Stream<std::ios>;

		ReadStream(std::istream& istream) : 
			m_istream(istream),
			Stream<std::ios>()
		{

		}

		template<typename T>
		inline ReadStream<std::ios>& operator >> (T& value);

		template<typename T>
		inline void advance_read_ptr();

	private:
		inline void read(std::uint8_t* data, std::uint32_t size);

	private:
		std::istream& m_istream;

		friend class Manager;
	};

	template<StreamDerivedFromStandard TStream>
	class ReadStream<TStream> : public TStream {
	public:
		using StreamType = TStream;

		ReadStream(std::istream& istream, const TStream& stream = TStream()) : 
			m_istream(istream),
			TStream(stream)
		{

		}

		template<typename T>
		inline ReadStream<TStream>& operator >> (T& value);

		template<typename T>
		inline void advance_read_ptr();

	private:
		inline void read(std::uint8_t* data, std::uint32_t size);

	private:
		std::istream& m_istream;

		friend class Manager;
	};

	class Manager {
	public:
		template<unsigned int UFieldId>
		struct Field {};

	public:
		template<typename T, typename TStream>
		inline static void write(const T& record, WriteStream<TStream>& stream) {
			static_assert(has_register<T>() || is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<TStream>::StreamType::get_channel())>() || std::is_fundamental_v<T>, "T does not declare ESR_BEGIN() or ESR_END(), it is not globally serializable nor fundamental");
			if constexpr(has_register<T>()) {
				internal_dispatch_write<T, TStream, T>(record, stream);
			}
			else if constexpr(is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<TStream>::StreamType::get_channel())>()) {
				if constexpr (!::ESR_API(Registry)<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<TStream>::StreamType::get_channel())>::is_none()) {
					::ESR_API(Registry)<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<TStream>::StreamType::get_channel())>::write(record, stream);
				}
			}
			else {
				stream.template advance_write_ptr<T>();
				stream.write((const std::uint8_t*)&record, sizeof(record));
			}
		}

		template<typename T, typename TStream>
		inline static void read(T& record, ReadStream<TStream>& stream) {
			static_assert(has_register<T>() || is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<TStream>::StreamType::get_channel())>() || std::is_fundamental_v<T>, "T does not declare ESR_BEGIN() or ESR_END(), it is not globally serializable nor fundamental");

			if constexpr(has_register<T>()) {
				internal_dispatch_read<T, TStream, T>(record, stream);
			}
			else if constexpr(is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<TStream>::StreamType::get_channel())>()) {
				if constexpr (!::ESR_API(Registry)<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<TStream>::StreamType::get_channel())>::is_none()) {
					::ESR_API(Registry)<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<TStream>::StreamType::get_channel())>::read(record, stream);
				 }
			}
			else {
				stream.template advance_read_ptr<T>();
				stream.read((std::uint8_t*)&record, sizeof(record));
			}
		}

		template<typename T, typename TStream = Stream<std::ios>>
		static consteval std::uint32_t get_sizeof() {
			static_assert(has_register<T>() || is_registered<T>() || std::is_fundamental_v<T>, "T does not declare ESR_BEGIN() or ESR_END(), it is not globally serializable nor fundamental");

			if constexpr (has_register<T>()) {
				return internal_dispatch_get_sizeof<TStream, T>();
			}
			else if constexpr (is_registered<T>()) {
				return ::ESR_API(Registry)<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(TStream::get_channel())>::get_sizeof();
			}
			else {
				return sizeof(T);
			}
		}

		template<typename T>
		static consteval bool has_register() {
			return has_begin<T>::value && has_end<T>::value;
		}

		template<typename T>
		static consteval bool is_registered() {
			return ::ESR_API(Registry)<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(std::string_view{""})>::is_valid();
		}

		template<typename T, typename TChannel>
		static consteval bool is_registered() {
			return ::ESR_API(Registry)<T, TChannel>::is_valid();
		}

		template<typename T, typename... TArgs>
		consteval static bool is_in_set() {
			if constexpr (sizeof...(TArgs)) {
				return is_in_set_internal<T, TArgs...>();
			}
			else {
				return false;
			}
		}

		template<typename T, typename TCurrent, typename... TArgs>
		consteval static bool is_in_set_internal() {
			T t;
			TCurrent current;

			if constexpr(t() == current()) {
				return true;
			}
			else if constexpr (sizeof...(TArgs)) {
				return is_in_set_internal<T, TArgs...>();
			}
			else {
				return false;
			}
		}


	private:
		template<typename, typename = std::void_t<>>
		struct has_begin : std::false_type{};

		template<typename T>
		struct has_begin<T, std::void_t<decltype(T::ESR_API(begin)())>> : std::true_type{};

		template<typename, typename = std::void_t<>>
		struct has_end : std::false_type{};

		template<typename T>
		struct has_end<T, std::void_t<decltype(T::ESR_API(end)())>> : std::true_type{};

	private:
		template<typename TRecord, typename... TBases>
		consteval static bool is_inheritance_valid() {
			if constexpr (sizeof...(TBases)) {
				return is_inheritance_valid_recursive<TRecord, TBases...>();
			}
			else {
				return true;
			}
		}

		template<typename TRecord, typename TCurrent, typename... TBases>
		consteval static bool is_inheritance_valid_recursive() {
			if constexpr (std::is_base_of_v<TCurrent, TRecord>) {
				if constexpr (sizeof...(TBases)) {
					return is_inheritance_valid_recursive<TRecord, TBases...>();
				}
				else {
					return true;
				}
			}
			else {
				return false;
			}
		}

		template<typename TRecord, typename TStream, typename TCurrent, typename... TBases>
		inline static void internal_dispatch_write(const TRecord& record, WriteStream<TStream>& stream) {
			if constexpr (has_register<TCurrent>()) {
				std::invoke([&record, &stream]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "ESR_BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						internal_dispatch_write<TRecord, TStream, TArgs...>(record, stream);
					}

				}, static_cast<TCurrent::ESR_API(Inheritance)::Bases*>(nullptr));

				stream.template advance_write_ptr<TCurrent>();
				internal_write<TCurrent, TStream, TCurrent::ESR_API(begin)() + 1>(*static_cast<const TCurrent*>(&record), stream);
			}
			else {
				write(*static_cast<const TCurrent*>(&record), stream);
			}
			if constexpr (sizeof...(TBases)) {
				internal_dispatch_write<TRecord, TStream, TBases...>(record, stream);
			}
		}

		template<typename T, typename TStream, unsigned int UFieldId>
		inline static void internal_write(const T& record, WriteStream<TStream>& stream) {
			const Field<UFieldId> field;
			if constexpr(T::ESR_API(end)() == UFieldId) {
				return;
			}
			else if constexpr (T::ESR_API(is_field_valid)(ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_VARIADIC(WriteStream<TStream>::StreamType::get_channel()), field)) {
				stream.template advance_write_ptr<T>();
				record.ESR_API(write)(stream, field);
				internal_write<T, TStream, UFieldId + 1>(record, stream);
			}
			else {
				internal_write<T, TStream, UFieldId + 1>(record, stream);
			}
		}

		template<typename TRecord, typename TStream, typename TCurrent, typename... TBases>
		inline static void internal_dispatch_read(TRecord& record, ReadStream<TStream>& stream) {
			if constexpr (has_register<TCurrent>()) {
				std::invoke([&record, &stream]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "ESR_BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						internal_dispatch_read<TRecord, TStream, TArgs...>(record, stream);
					}

				}, static_cast<TCurrent::ESR_API(Inheritance)::Bases*>(nullptr));

				stream.template advance_read_ptr<TCurrent>();
				internal_read<TCurrent, TStream, TCurrent::ESR_API(begin)() + 1>(*static_cast<TCurrent*>(&record), stream);
			}
			else {
				read(*static_cast<TCurrent*>(&record), stream);
			}
			if constexpr (sizeof...(TBases)) {
				internal_dispatch_read<TRecord, TStream, TBases...>(record, stream);
			}
		}

		template<typename T, typename TStream, unsigned int UFieldId>
		inline static void internal_read(T& record, ReadStream<TStream>& stream) {
			const Field<UFieldId> field;
			if constexpr(T::ESR_API(end)() == UFieldId) {
				return;
			}
			else if constexpr (T::ESR_API(is_field_valid)(ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_VARIADIC(ReadStream<TStream>::StreamType::get_channel()), field)) {
				stream.template advance_read_ptr<T>();
				record.ESR_API(read)(stream, field);
				internal_read<T, TStream, UFieldId + 1>(record, stream);
			}
			else {
				internal_read<T, TStream, UFieldId + 1>(record, stream);
			}
		}

		template<typename TStream, typename TCurrent, typename... TBases>
		static consteval std::uint32_t internal_dispatch_get_sizeof() {
			if constexpr (has_register<TCurrent>()) {
				const std::uint32_t partial_sizeof = std::invoke([]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "ESR_BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						return internal_dispatch_get_sizeof<TStream, TArgs...>();
					}
					else {
						return 0;
					}
				}, static_cast<TCurrent::ESR_API(Inheritance)::Bases*>(nullptr));

				const std::uint32_t return_sizeof = partial_sizeof + internal_get_sizeof<TStream, TCurrent, TCurrent::ESR_API(begin)() + 1>();

				if constexpr (sizeof... (TBases)) {
					return return_sizeof + internal_dispatch_get_sizeof<TStream, TBases...>();
				}
				else {
					return return_sizeof;
				}
			}
			else if constexpr (sizeof...(TBases)) {
				return get_sizeof<TCurrent>() + internal_dispatch_get_sizeof<TStream, TBases...>();
			}
			else {
				return get_sizeof<TCurrent>();
			}
		}

		template<typename TStream, typename T, std::uint32_t UFieldId>
		static consteval std::uint32_t internal_get_sizeof() {
			const Field<UFieldId> field;
			if constexpr(T::ESR_API(end()) == UFieldId) {
				return 0;
			}
			else if constexpr (T::ESR_API(is_field_valid)(ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_VARIADIC(TStream::get_channel()), field)) {
				return T::ESR_API(get_sizeof)(field) + internal_get_sizeof<TStream, T, UFieldId + 1>();
			}
			else {
				return internal_get_sizeof<TStream, T, UFieldId + 1>();
			}
		}

	};

	template<typename TStream> 
	template<typename T>
	WriteStream<TStream>& WriteStream<TStream>::operator << (const T& value) {
		static_assert(Manager::has_register<T>() || Manager::is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<TStream>::StreamType::get_channel())>() || std::is_fundamental_v<T>, "T does not declare ESR_BEGIN() or ESR_END(), it is not globally serializable nor fundamental");
		Manager::write(value, *this);

		return *this;
	}

	template<typename TStream> 
	template<typename T>
	void WriteStream<TStream>::advance_write_ptr() {
		std::uint32_t mod = reinterpret_cast<std::uintptr_t>(m_write_ptr) % TStream::template get_alignof<T>();
		m_write_ptr += mod;
		assert((m_write_ptr - Stream<TStream>::get_handle()) <= Stream<TStream>::get_size());
	}

	template<typename TStream> 
	void WriteStream<TStream>::write(const std::uint8_t* data, std::uint32_t size) {
		assert((m_write_ptr - Stream<TStream>::get_handle() + size) < Stream<TStream>::get_size());
		std::memcpy(m_write_ptr, data, size);
		m_write_ptr += size;
	}

	template<typename T>
	WriteStream<std::ios>& WriteStream<std::ios>::operator << (const T& value) {
		static_assert(Manager::has_register<T>() || Manager::is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<std::ios>::StreamType::get_channel())>() || std::is_fundamental_v<T>, "T does not declare ESR_BEGIN() or ESR_END(), it is not globally serializable nor fundamental");
		Manager::write(value, *this);

		return *this;
	}

	template<typename T>
	void WriteStream<std::ios>::advance_write_ptr() {
		std::uint32_t mod = static_cast<std::uint32_t>(m_ostream.tellp()) % StreamType::template get_alignof<T>();
		m_ostream.seekp(static_cast<std::uint32_t>(m_ostream.tellp()) + mod);
	}

	void WriteStream<std::ios>::write(const std::uint8_t* data, std::uint32_t size) {
		m_ostream.write(reinterpret_cast<const char*>(data), size);
	}

	template<StreamDerivedFromStandard TStream>
	template<typename T>
	WriteStream<TStream>& WriteStream<TStream>::operator << (const T& value) {
		static_assert(Manager::has_register<T>() || Manager::is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<TStream>::StreamType::get_channel())>() || std::is_fundamental_v<T>, "T does not declare ESR_BEGIN() or ESR_END(), it is not globally serializable nor fundamental");
		Manager::write(value, *this);

		return *this;
	}

	template<StreamDerivedFromStandard TStream>
	template<typename T>
	void WriteStream<TStream>::advance_write_ptr() {
		std::uint32_t mod = static_cast<std::uint32_t>(m_ostream.tellp()) % StreamType::template get_alignof<T>();
		m_ostream.seekp(static_cast<std::uint32_t>(m_ostream.tellp()) + mod);
	}

	template<StreamDerivedFromStandard TStream>
	void WriteStream<TStream>::write(const std::uint8_t* data, std::uint32_t size) {
		m_ostream.write(reinterpret_cast<const char*>(data), size);
	}

	template<typename TStream> 
	template<typename T>
	ReadStream<TStream>& ReadStream<TStream>::operator >> (T& value) {
		static_assert(Manager::has_register<T>() || Manager::is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<TStream>::StreamType::get_channel())>() || std::is_fundamental_v<T>, "T does not declare ESR_BEGIN() or ESR_END(), it is not globally serializable nor fundamental");
		Manager::read(value, *this);

		return *this;
	}

	template<typename TStream> 
	template<typename T>
	void ReadStream<TStream>::advance_read_ptr() {
		std::uint32_t mod = reinterpret_cast<std::uintptr_t>(m_read_ptr) % TStream::template get_alignof<T>();
		m_read_ptr += mod;
		assert((m_read_ptr - Stream<TStream>::get_handle()) <= Stream<TStream>::get_size());
	}

	template<typename TStream> 
	void ReadStream<TStream>::read(std::uint8_t* data, std::uint32_t size) {
		assert((m_read_ptr - Stream<TStream>::get_handle() + size) < Stream<TStream>::get_size());
		std::memcpy(data, m_read_ptr, size);
		m_read_ptr += size;
	}

	template<typename T>
	ReadStream<std::ios>& ReadStream<std::ios>::operator >> (T& value) {
		static_assert(Manager::has_register<T>() || Manager::is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<std::ios>::StreamType::get_channel())>() || std::is_fundamental_v<T>, "T does not declare ESR_BEGIN() or ESR_END(), it is not globally serializable nor fundamental");
		Manager::read(value, *this);

		return *this;
	}

	template<typename T>
	void ReadStream<std::ios>::advance_read_ptr() {
		std::uint32_t mod = static_cast<std::uint32_t>(m_istream.tellg()) % StreamType::template get_alignof<T>();
		m_istream.seekg(static_cast<std::uint32_t>(m_istream.tellg()) + mod);
	}

	void ReadStream<std::ios>::read(std::uint8_t* data, std::uint32_t size) {
		m_istream.read(reinterpret_cast<char*>(data), size);
	}

	template<StreamDerivedFromStandard TStream>
	template<typename T>
	ReadStream<TStream>& ReadStream<TStream>::operator >> (T& value) {
		static_assert(Manager::has_register<T>() || Manager::is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<TStream>::StreamType::get_channel())>() || std::is_fundamental_v<T>, "T does not declare ESR_BEGIN() or ESR_END(), it is not globally serializable nor fundamental");
		Manager::read(value, *this);

		return *this;
	}

	template<StreamDerivedFromStandard TStream>
	template<typename T>
	void ReadStream<TStream>::advance_read_ptr() {
		std::uint32_t mod = static_cast<std::uint32_t>(m_istream.tellg()) % StreamType::template get_alignof<T>();
		m_istream.seekg(static_cast<std::uint32_t>(m_istream.tellg()) + mod);
	}

	template<StreamDerivedFromStandard TStream>
	void ReadStream<TStream>::read(std::uint8_t* data, std::uint32_t size) {
		m_istream.read(reinterpret_cast<char*>(data), size);
	}
}

#endif	// EASER_EASER_H
