#ifndef EASER_STREAMER_H
#define EASER_STREAMER_H

#include "easer/base.h"
#include <limits>
#include <fstream>
#include <functional>
#include <cassert>

#define REGISTER(type, ...) \
	template<> \
	class Registry<std::decay<type>> { \
	public: \
		static consteval bool is_valid() { \
			return true; \
		} \
	private: \
		inline static void write(type& v, std::ostream& stream) { \
			internal_write(v, stream, FIELD_NAME_VARIADIC_TO_FIELD_ACCESS_VARIADIC(v, __VA_ARGS__)); \
		} \
		template<typename TStream> \
		inline static void write(type& v, ::easer::Stream<TStream>& stream) { \
			stream.template advance_write_ptr<type>(); \
			internal_write(v, stream, FIELD_NAME_VARIADIC_TO_FIELD_ACCESS_VARIADIC(v, __VA_ARGS__)); \
		} \
		inline static void read(type& v, std::istream& stream) { \
			internal_read(v, stream, FIELD_NAME_VARIADIC_TO_FIELD_ACCESS_VARIADIC(v, __VA_ARGS__)); \
		} \
		template<typename TStream> \
		inline static void read(type& v, ::easer::Stream<TStream>& stream) { \
			stream.template advance_read_ptr<type>(); \
			internal_read(v, stream, FIELD_NAME_VARIADIC_TO_FIELD_ACCESS_VARIADIC(v, __VA_ARGS__)); \
		} \
		static consteval std::uint32_t get_sizeof() { \
			return internal_get_sizeof<FIELD_NAME_VARIADIC_TO_FIELD_TYPE_VARIADIC(type, __VA_ARGS__)>(); \
		} \
		template<typename TCurrent, typename... TArgs> \
		inline static void internal_write(type& name, std::ostream& stream, TCurrent& write_ptr, TArgs&... args) { \
			::easer::Streamer::write<TCurrent>(write_ptr, stream); \
			if constexpr(sizeof...(TArgs)) { \
				internal_write(name, stream, args...); \
			} \
		} \
		template<typename TCurrent, typename TStream, typename... TArgs> \
		inline static void internal_write(type& name, ::easer::Stream<TStream>& stream, TCurrent& write_ptr, TArgs&... args) { \
			stream.template advance_write_ptr<TCurrent>(); \
			::easer::Streamer::write<TCurrent>(write_ptr, stream); \
			if constexpr(sizeof...(TArgs)) { \
				internal_write(name, stream, args...); \
			} \
		} \
		template<typename TCurrent, typename... TArgs> \
		inline static void internal_read(type& name, std::istream& stream, TCurrent& write_ptr, TArgs&... args) { \
			::easer::Streamer::read<TCurrent>(write_ptr, stream); \
			if constexpr(sizeof...(TArgs)) { \
				internal_read(name, stream, args...); \
			} \
		} \
		template<typename TCurrent, typename TStream, typename... TArgs> \
		inline static void internal_read(type& name, ::easer::Stream<TStream>& stream, TCurrent& write_ptr, TArgs&... args) { \
			::easer::Streamer::read<TCurrent>(write_ptr, stream); \
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
		friend class ::easer::Streamer; \
	};

#define BEGIN(...) \
	struct Inheritance { \
		using Bases = std::tuple<__VA_ARGS__>; \
	}; \
	static consteval unsigned int begin() { \
		const unsigned int begin_id = __COUNTER__; \
		static_assert(begin_id != std::numeric_limits<unsigned int>::max()); \
		return begin_id; \
	} \
	template<typename TField> \
	static consteval bool is_field_valid(TField) { \
		return false; \
	} \
	template<typename T, typename V> \
	friend class has_begin;

#define FIELD(field_type, field_name) \
	static consteval bool is_field_valid(const ::easer::Streamer::Field<__COUNTER__>) { \
		static_assert(::easer::Streamer::has_register<field_type>() || ::is_registered<field_type>() || std::is_trivial_v<field_type>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial"); \
		return true; \
	} \
	inline void write(std::ostream& stream, const ::easer::Streamer::Field<__COUNTER__ - 1>) { \
		::easer::Streamer::write(field_name, stream); \
	} \
	template<typename TStream> \
	inline void write(::easer::Stream<TStream>& stream, const ::easer::Streamer::Field<__COUNTER__ - 2>) { \
		::easer::Streamer::write(field_name, stream); \
	} \
	inline void read(std::istream& stream, const ::easer::Streamer::Field<__COUNTER__ - 3>) { \
		::easer::Streamer::read(field_name, stream); \
	} \
	template<typename TStream> \
	inline void read(::easer::Stream<TStream>& stream, const ::easer::Streamer::Field<__COUNTER__ - 4>) { \
		::easer::Streamer::read(field_name, stream); \
	} \
	static consteval std::uint32_t get_sizeof(const ::easer::Streamer::Field<__COUNTER__ - 5>) { \
		return ::easer::Streamer::get_sizeof<field_type>(); \
	} \
	field_type field_name

#define END() \
	static consteval unsigned int end() { \
		return __COUNTER__; \
	} \
	friend class ::easer::Streamer; \
	template<typename T, typename V> \
	friend class has_end;

template<typename T>
struct Registry {
	static consteval bool is_valid() {
		return false;
	}
};

template<typename T>
consteval bool is_registered() {
	return ::Registry<T>::is_valid();
}

namespace easer {
	class Streamer;

	template<typename TStream>
	class Stream {
	public:
		Stream(std::uint8_t* handle, std::uint32_t size) : m_handle(handle), m_size(size), m_write_ptr(handle), m_read_ptr(handle) {
			static_assert(std::is_base_of_v<Stream<TStream>, TStream>);
		}

		~Stream() {
		}

		template<typename T>
		inline Stream& operator << (T& value);

		template<typename T>
		inline Stream& operator >> (T& value);

		template<typename T>
		inline static std::uint32_t get_alignof() {
			return alignof(T);
		}

		inline const std::uint8_t* get_handle() const {
			return m_handle;
		}

		inline const std::uint8_t* get_write_ptr() const {
			return m_write_ptr;
		}

		inline const std::uint8_t* get_read_ptr() const {
			return m_read_ptr;
		}

		inline std::uint32_t get_size() const {
			return m_size;
		}

	private:
		template<typename T>
		inline void advance_write_ptr();
		template<typename T>
		inline void advance_read_ptr();

		inline void write(const char* data, std::uint32_t size);
		inline void read(char* data, std::uint32_t size);

	private:
		std::uint8_t* m_handle;
		std::uint8_t* m_write_ptr;
		std::uint8_t* m_read_ptr;
		std::uint32_t m_size;

		friend class Streamer;
	};

	class Streamer {
	public:
		template<unsigned int UFieldId>
		struct Field {};

	public:
		template<typename T>
		inline static void write(T& record, std::ostream& stream) {
			static_assert(has_register<T>() || ::is_registered<T>() || std::is_trivial_v<T>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial");
			if constexpr(has_register<T>()) {
				internal_dispatch_write<T, T>(record, stream);
			}
			else if constexpr(::is_registered<T>()) {
				::Registry<T>::write(record, stream);
			}
			else {
				stream.write((const char*)&record, sizeof(record));
			}
		}

		template<typename T, typename TStream>
		inline static void write(T& record, Stream<TStream>& stream) {
			static_assert(has_register<T>() || ::is_registered<T>() || std::is_trivial_v<T>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial");
			if constexpr(has_register<T>()) {
				internal_dispatch_write<T, TStream, T>(record, stream);
			}
			else if constexpr(::is_registered<T>()) {
				::Registry<T>::write(record, stream);
			}
			else {
				stream.template advance_write_ptr<T>();
				stream.write((const char*)&record, sizeof(record));
			}
		}

		template<typename T>
		inline static void read(T& record, std::istream& stream) {
			static_assert(has_register<T>() || ::is_registered<T>() || std::is_trivial_v<T>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial");

			if constexpr(has_register<T>()) {
				internal_dispatch_read<T, T>(record, stream);
			}
			else if constexpr(::is_registered<T>()) {
				::Registry<T>::read(record, stream);
			}
			else {
				stream.read((char*)&record, sizeof(record));
			}
		}

		template<typename T, typename TStream>
		inline static void read(T& record, Stream<TStream>& stream) {
			static_assert(has_register<T>() || ::is_registered<T>() || std::is_trivial_v<T>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial");

			if constexpr(has_register<T>()) {
				internal_dispatch_read<T, TStream, T>(record, stream);
			}
			else if constexpr(::is_registered<T>()) {
				::Registry<T>::read(record, stream);
			}
			else {
				stream.template advance_read_ptr<T>();
				stream.read((char*)&record, sizeof(record));
			}
		}

		template<typename T>
		static consteval std::uint32_t get_sizeof() {
			static_assert(has_register<T>() || ::is_registered<T>() || std::is_trivial_v<T>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial");

			if constexpr (has_register<T>()) {
				return internal_dispatch_get_sizeof<T>();
			}
			else if constexpr (::is_registered<T>()) {
				return ::Registry<T>::get_sizeof();
			}
			else {
				return sizeof(T);
			}
		}

		template<typename T>
		static consteval bool has_register() {
			return has_begin<T>::value && has_end<T>::value;
		}

	private:
		template<typename, typename = std::void_t<>>
		struct has_begin : std::false_type{};

		template<typename T>
		struct has_begin<T, std::void_t<decltype(T::begin())>> : std::true_type{};

		template<typename, typename = std::void_t<>>
		struct has_end : std::false_type{};

		template<typename T>
		struct has_end<T, std::void_t<decltype(T::end())>> : std::true_type{};

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

		template<typename TRecord, typename TCurrent, typename... TBases>
		inline static void internal_dispatch_write(TRecord& record, std::ostream& stream) {
			if constexpr (has_register<TCurrent>()) {
				std::invoke([&record, &stream]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						internal_dispatch_write<TRecord, TArgs...>(record, stream);
					}

				}, static_cast<TCurrent::Inheritance::Bases*>(nullptr));
				internal_write<TCurrent, TCurrent::begin() + 1>(*static_cast<TCurrent*>(&record), stream);
			}
			else {
				write(*static_cast<TCurrent*>(&record), stream);
			}
			if constexpr (sizeof...(TBases)) {
				internal_dispatch_write<TRecord, TBases...>(record, stream);
			}
		}

		template<typename TRecord, typename TStream, typename TCurrent, typename... TBases>
		inline static void internal_dispatch_write(TRecord& record, Stream<TStream>& stream) {
			if constexpr (has_register<TCurrent>()) {
				std::invoke([&record, &stream]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						internal_dispatch_write<TRecord, TStream, TArgs...>(record, stream);
					}

				}, static_cast<TCurrent::Inheritance::Bases*>(nullptr));

				stream.template advance_write_ptr<TCurrent>();
				internal_write<TCurrent, TStream, TCurrent::begin() + 1>(*static_cast<TCurrent*>(&record), stream);
			}
			else {
				write(*static_cast<TCurrent*>(&record), stream);
			}
			if constexpr (sizeof...(TBases)) {
				internal_dispatch_write<TRecord, TStream, TBases...>(record, stream);
			}
		}

		template<typename T, unsigned int UFieldId>
		inline static void internal_write(T& record, std::ostream& stream) {
			const Field<UFieldId> field;
			if constexpr(T::end() == UFieldId) {
				return;
			}
			else if constexpr (T::is_field_valid(field)) {
				record.write(stream, field);
				internal_write<T, UFieldId + 1>(record, stream);
			}
			else {
				internal_write<T, UFieldId + 1>(record, stream);
			}
		}

		template<typename T, typename TStream, unsigned int UFieldId>
		inline static void internal_write(T& record, Stream<TStream>& stream) {
			const Field<UFieldId> field;
			if constexpr(T::end() == UFieldId) {
				return;
			}
			else if constexpr (T::is_field_valid(field)) {
				stream.template advance_write_ptr<T>();
				record.write(stream, field);
				internal_write<T, TStream, UFieldId + 1>(record, stream);
			}
			else {
				internal_write<T, TStream, UFieldId + 1>(record, stream);
			}
		}

		template<typename TRecord, typename TCurrent, typename... TBases>
		inline static void internal_dispatch_read(TRecord& record, std::istream& stream) {
			if constexpr (has_register<TCurrent>()) {
				std::invoke([&record, &stream]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						internal_dispatch_read<TRecord, TArgs...>(record, stream);
					}

				}, static_cast<TCurrent::Inheritance::Bases*>(nullptr));
				internal_read<TCurrent, TCurrent::begin() + 1>(*static_cast<TCurrent*>(&record), stream);
			}
			else {
				read(*static_cast<TCurrent*>(&record), stream);
			}
			if constexpr (sizeof...(TBases)) {
				internal_dispatch_read<TRecord, TBases...>(record, stream);
			}
		}

		template<typename TRecord, typename TStream, typename TCurrent, typename... TBases>
		inline static void internal_dispatch_read(TRecord& record, Stream<TStream>& stream) {
			if constexpr (has_register<TCurrent>()) {
				std::invoke([&record, &stream]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						internal_dispatch_read<TRecord, TArgs...>(record, stream);
					}

				}, static_cast<TCurrent::Inheritance::Bases*>(nullptr));

				stream.template advance_read_ptr<TCurrent>();
				internal_read<TCurrent, TStream, TCurrent::begin() + 1>(*static_cast<TCurrent*>(&record), stream);
			}
			else {
				read(*static_cast<TCurrent*>(&record), stream);
			}
			if constexpr (sizeof...(TBases)) {
				internal_dispatch_read<TRecord, TBases...>(record, stream);
			}
		}

		template<typename T, unsigned int UFieldId>
		inline static void internal_read(T& record, std::istream& stream) {
			const Field<UFieldId> field;
			if constexpr(T::end() == UFieldId) {
				return;
			}
			else if constexpr (T::is_field_valid(field)) {
				record.read(stream, field);
				internal_read<T, UFieldId + 1>(record, stream);
			}
			else {
				internal_read<T, UFieldId + 1>(record, stream);
			}
		}

		template<typename T, typename TStream, unsigned int UFieldId>
		inline static void internal_read(T& record, Stream<TStream>& stream) {
			const Field<UFieldId> field;
			if constexpr(T::end() == UFieldId) {
				return;
			}
			else if constexpr (T::is_field_valid(field)) {
				stream.template advance_read_ptr<T>();
				record.read(stream, field);
				internal_read<T, TStream, UFieldId + 1>(record, stream);
			}
			else {
				internal_read<T, TStream, UFieldId + 1>(record, stream);
			}
		}

		template<typename TCurrent, typename... TBases>
		static consteval std::uint32_t internal_dispatch_get_sizeof() {
			if constexpr (has_register<TCurrent>()) {
				const std::uint32_t partial_sizeof = std::invoke([]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						return internal_dispatch_get_sizeof<TArgs...>();
					}
					else {
						return 0;
					}
				}, static_cast<TCurrent::Inheritance::Bases*>(nullptr));

				const std::uint32_t return_sizeof = partial_sizeof + internal_get_sizeof<TCurrent, TCurrent::begin() + 1>();

				if constexpr (sizeof... (TBases)) {
					return return_sizeof + internal_dispatch_get_sizeof<TBases...>();
				}
				else {
					return return_sizeof;
				}
			}
			else if constexpr (sizeof...(TBases)) {
				return get_sizeof<TCurrent>() + internal_dispatch_get_sizeof<TBases...>();
			}
			else {
				return get_sizeof<TCurrent>();
			}
		}

		template<typename T, std::uint32_t UFieldId>
		static consteval std::uint32_t internal_get_sizeof() {
			const Field<UFieldId> field;
			if constexpr(T::end() == UFieldId) {
				return 0;
			}
			else if constexpr (T::is_field_valid(field)) {
				return T::get_sizeof(field) + internal_get_sizeof<T, UFieldId + 1>();
			}
			else {
				return internal_get_sizeof<T, UFieldId + 1>();
			}
		}

	};

	template<typename TStream> 
	template<typename T>
	Stream<TStream>& Stream<TStream>::operator << (T& value) {
		static_assert(Streamer::has_register<T>() || ::is_registered<T>() || std::is_trivial_v<T>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial");
		Streamer::write(value, *this);

		return *this;
	}

	template<typename TStream> 
	template<typename T>
	Stream<TStream>& Stream<TStream>::operator >> (T& value) {
		static_assert(Streamer::has_register<T>() || ::is_registered<T>() || std::is_trivial_v<T>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial");
		Streamer::read(value, *this);

		return *this;
	}

	template<typename TStream> 
	template<typename T>
	void Stream<TStream>::advance_write_ptr() {
		std::uint32_t mod = reinterpret_cast<std::uintptr_t>(m_write_ptr) % TStream::template get_alignof<T>();
		m_write_ptr += TStream::template get_alignof<T>() - mod;
		assert((m_write_ptr - m_handle) <= m_size);
	}

	template<typename TStream> 
	template<typename T>
	void Stream<TStream>::advance_read_ptr() {
		std::uint32_t mod = reinterpret_cast<std::uintptr_t>(m_read_ptr) % TStream::template get_alignof<T>();
		m_read_ptr += TStream::template get_alignof<T>() - mod;
		assert((m_read_ptr - m_handle) <= m_size);
	}

	template<typename TStream> 
	inline void Stream<TStream>::write(const char* data, std::uint32_t size) {
		assert((m_write_ptr - m_handle + size) < m_size);
		std::memcpy(m_write_ptr, data, size);
		m_write_ptr += size;
	}

	template<typename TStream> 
	inline void Stream<TStream>::read(char* data, std::uint32_t size) {
		assert((m_read_ptr - m_handle + size) < m_size);
		std::memcpy(data, m_read_ptr, size);
		m_read_ptr += size;
	}
}

#endif	// EASER_STREAMER_H
