#ifndef EASER_STREAMER_H
#define EASER_STREAMER_H

#include "easer/base.h"
#include <limits>
#include <fstream>
#include <functional>
#include <type_traits>
#include <cassert>

#define EASER_API(name) __easer__##name

#define REGISTER(type, ...) \
	class EASER_API(Registry<type>) { \
	public: \
		static consteval bool is_valid() { \
			return true; \
		} \
	private: \
		template<typename TStream> \
		inline static void write(const type& v, ::easer::WriteStream<TStream>& stream) { \
			stream.template advance_write_ptr<type>(); \
			internal_write(v, stream, FIELD_NAME_VARIADIC_TO_FIELD_ACCESS_VARIADIC(v, __VA_ARGS__)); \
		} \
		template<typename TStream> \
		inline static void read(type& v, ::easer::ReadStream<TStream>& stream) { \
			stream.template advance_read_ptr<type>(); \
			internal_read(v, stream, FIELD_NAME_VARIADIC_TO_FIELD_ACCESS_VARIADIC(v, __VA_ARGS__)); \
		} \
		static consteval std::uint32_t get_sizeof() { \
			return internal_get_sizeof<FIELD_NAME_VARIADIC_TO_FIELD_TYPE_VARIADIC(type, __VA_ARGS__)>(); \
		} \
		template<typename TCurrent, typename TStream, typename... TArgs> \
		inline static void internal_write(const type& name, ::easer::WriteStream<TStream>& stream, TCurrent& write_ptr, TArgs&... args) { \
			stream.template advance_write_ptr<TCurrent>(); \
			::easer::Streamer::write<TCurrent>(write_ptr, stream); \
			if constexpr(sizeof...(TArgs)) { \
				internal_write(name, stream, args...); \
			} \
		} \
		template<typename TCurrent, typename TStream, typename... TArgs> \
		inline static void internal_read(type& name, ::easer::ReadStream<TStream>& stream, TCurrent& write_ptr, TArgs&... args) { \
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
	}; \


#define REGISTER_PROC(type, type_name, stream_name, write_func_body, read_func_body, get_sizeof_func_body) \
	class EASER_API(Registry<type>) { \
	public: \
		static consteval bool is_valid() { \
			return true; \
		} \
	private: \
		template<typename EASER_API(TStream)> \
		inline static void write(const type& type_name, ::easer::WriteStream<EASER_API(TStream)>& stream_name) { \
			stream.template advance_write_ptr<type>(); \
			write_func_body; \
		} \
		template<typename EASER_API(TStream)> \
		inline static void read(type& type_name, ::easer::ReadStream<EASER_API(TStream)>& stream_name) { \
			stream.template advance_read_ptr<type>(); \
			read_func_body; \
		} \
		static consteval std::uint32_t get_sizeof(type& type_name) { \
			get_sizeof_func_body; \
		} \
		friend class ::easer::Streamer; \
	}; \

#define BEGIN(...) \
	struct EASER_API(Inheritance) { \
		using Bases = std::tuple<__VA_ARGS__>; \
	}; \
	static consteval unsigned int EASER_API(begin)() { \
		const unsigned int begin_id = __COUNTER__; \
		static_assert(begin_id != std::numeric_limits<unsigned int>::max()); \
		return begin_id; \
	} \
	template<typename TField> \
	static consteval bool EASER_API(is_field_valid)(TField) { \
		return false; \
	} \
	template<typename T, typename V> \
	friend class ::easer::Streamer::has_begin;

#define FIELD(field_type, field_name) \
	static consteval bool EASER_API(is_field_valid)(const ::easer::Streamer::Field<__COUNTER__>) { \
		static_assert(::easer::Streamer::has_register<field_type>() || ::easer::Streamer::is_registered<field_type>() || std::is_trivial_v<field_type>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial"); \
		return true; \
	} \
	template<typename TStream> \
	inline void EASER_API(write)(::easer::WriteStream<TStream>& stream, const ::easer::Streamer::Field<__COUNTER__ - 1>) const { \
		::easer::Streamer::write(field_name, stream); \
	} \
	template<typename TStream> \
	inline void EASER_API(read)(::easer::ReadStream<TStream>& stream, const ::easer::Streamer::Field<__COUNTER__ - 2>) { \
		::easer::Streamer::read(field_name, stream); \
	} \
	static consteval std::uint32_t EASER_API(get_sizeof)(const ::easer::Streamer::Field<__COUNTER__ - 3>) { \
		return ::easer::Streamer::get_sizeof<field_type>(); \
	} \
	field_type field_name

#define END() \
	static consteval unsigned int EASER_API(end()) { \
		return __COUNTER__; \
	} \
	friend class ::easer::Streamer; \
	template<typename T, typename V> \
	friend class easer::Streamer::has_end; \

template<typename T>
struct EASER_API(Registry) {
	static consteval bool is_valid() {
		return false;
	}
};

namespace easer {
	class Streamer;

	template<typename TStream>
	class Stream {
	public:
		template<typename T>
		inline static std::uint32_t get_alignof() {
			return alignof(T);
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

	protected:
		Stream() = default;
	};

	template<typename TStream = std::ios>
	class WriteStream : public Stream<TStream> {
	public:
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
		inline void write(const char* data, std::uint32_t size);

	private:
		std::uint8_t* m_write_ptr;

		friend class Streamer;
	};

	template<typename TStream = std::ios>
	class ReadStream : public Stream<TStream> {
	public:
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
		inline void read(char* data, std::uint32_t size);

	private:
		std::uint8_t* m_read_ptr;

		friend class Streamer;
	};

	template<typename TStream>
		requires std::is_base_of_v<Stream<std::ios>, TStream> || std::is_same_v<std::ios, TStream>
	class WriteStream<TStream> : public Stream<TStream> {
	public:
		using StreamType = std::conditional_t<std::is_same_v<std::ios, TStream>, Stream<TStream>, TStream>;

		WriteStream(std::ostream& ostream) : 
			Stream<TStream>(),
			m_ostream(ostream) 
		{
			
		}

		~WriteStream() {
		}

		template<typename T>
		inline WriteStream<TStream>& operator << (const T& value);

		template<typename T>
		inline void advance_write_ptr();

	private:
		inline void write(const char* data, std::uint32_t size);

	private:
		std::ostream& m_ostream;

		friend class Streamer;
	};

	template<typename TStream>
		requires std::is_base_of_v<Stream<std::ios>, TStream> || std::is_same_v<std::ios, TStream>
	class ReadStream<TStream> : public Stream<TStream> {
	public:
		using StreamType = std::conditional_t<std::is_same_v<std::ios, TStream>, Stream<TStream>, TStream>;

		ReadStream(std::istream& istream) : 
			Stream<TStream>(),
			m_istream(istream) 
		{

		}

		template<typename T>
		inline ReadStream<TStream>& operator >> (T& value);

		template<typename T>
		inline void advance_read_ptr();

	private:
		inline void read(char* data, std::uint32_t size);

	private:
		std::istream& m_istream;

		friend class Streamer;
	};

	class Streamer {
	public:
		template<unsigned int UFieldId>
		struct Field {};

	public:
		template<typename T, typename TStream>
		inline static void write(const T& record, WriteStream<TStream>& stream) {
			static_assert(has_register<T>() || is_registered<T>() || std::is_trivial_v<T>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial");
			if constexpr(has_register<T>()) {
				internal_dispatch_write<T, TStream, T>(record, stream);
			}
			else if constexpr(is_registered<T>()) {
				::EASER_API(Registry<T>)::write(record, stream);
			}
			else {
				stream.template advance_write_ptr<T>();
				stream.write((const char*)&record, sizeof(record));
			}
		}

		template<typename T, typename TStream>
		inline static void read(T& record, ReadStream<TStream>& stream) {
			static_assert(has_register<T>() || is_registered<T>() || std::is_trivial_v<T>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial");

			if constexpr(has_register<T>()) {
				internal_dispatch_read<T, TStream, T>(record, stream);
			}
			else if constexpr(is_registered<T>()) {
				::EASER_API(Registry<T>)::read(record, stream);
			}
			else {
				stream.template advance_read_ptr<T>();
				stream.read((char*)&record, sizeof(record));
			}
		}

		template<typename T>
		static consteval std::uint32_t get_sizeof() {
			static_assert(has_register<T>() || is_registered<T>() || std::is_trivial_v<T>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial");

			if constexpr (has_register<T>()) {
				return internal_dispatch_get_sizeof<T>();
			}
			else if constexpr (is_registered<T>()) {
				return ::EASER_API(Registry<T>)::get_sizeof();
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
			return ::EASER_API(Registry<T>)::is_valid();
		}

	private:
		template<typename, typename = std::void_t<>>
		struct has_begin : std::false_type{};

		template<typename T>
		struct has_begin<T, std::void_t<decltype(T::EASER_API(begin)())>> : std::true_type{};

		template<typename, typename = std::void_t<>>
		struct has_end : std::false_type{};

		template<typename T>
		struct has_end<T, std::void_t<decltype(T::EASER_API(end)())>> : std::true_type{};

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
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						internal_dispatch_write<TRecord, TStream, TArgs...>(record, stream);
					}

				}, static_cast<TCurrent::EASER_API(Inheritance)::Bases*>(nullptr));

				stream.template advance_write_ptr<TCurrent>();
				internal_write<TCurrent, TStream, TCurrent::EASER_API(begin)() + 1>(*static_cast<const TCurrent*>(&record), stream);
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
			if constexpr(T::EASER_API(end)() == UFieldId) {
				return;
			}
			else if constexpr (T::EASER_API(is_field_valid)(field)) {
				stream.template advance_write_ptr<T>();
				record.EASER_API(write)(stream, field);
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
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						internal_dispatch_read<TRecord, TStream, TArgs...>(record, stream);
					}

				}, static_cast<TCurrent::EASER_API(Inheritance)::Bases*>(nullptr));

				stream.template advance_read_ptr<TCurrent>();
				internal_read<TCurrent, TStream, TCurrent::EASER_API(begin)() + 1>(*static_cast<TCurrent*>(&record), stream);
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
			if constexpr(T::EASER_API(end)() == UFieldId) {
				return;
			}
			else if constexpr (T::EASER_API(is_field_valid)(field)) {
				stream.template advance_read_ptr<T>();
				record.EASER_API(read)(stream, field);
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
				}, static_cast<TCurrent::EASER_API(Inheritance)::Bases*>(nullptr));

				const std::uint32_t return_sizeof = partial_sizeof + internal_get_sizeof<TCurrent, TCurrent::EASER_API(begin)() + 1>();

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
			if constexpr(T::EASER_API(end()) == UFieldId) {
				return 0;
			}
			else if constexpr (T::EASER_API(is_field_valid)(field)) {
				return T::EASER_API(get_sizeof)(field) + internal_get_sizeof<T, UFieldId + 1>();
			}
			else {
				return internal_get_sizeof<T, UFieldId + 1>();
			}
		}

	};

	template<typename TStream> 
	template<typename T>
	WriteStream<TStream>& WriteStream<TStream>::operator << (const T& value) {
		static_assert(Streamer::has_register<T>() || Streamer::is_registered<T>() || std::is_trivial_v<T>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial");
		Streamer::write(value, *this);

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
	void WriteStream<TStream>::write(const char* data, std::uint32_t size) {
		assert((m_write_ptr - Stream<TStream>::get_handle() + size) < Stream<TStream>::get_size());
		std::memcpy(m_write_ptr, data, size);
		m_write_ptr += size;
	}

	template<typename TStream> 
	template<typename T>
	ReadStream<TStream>& ReadStream<TStream>::operator >> (T& value) {
		static_assert(Streamer::has_register<T>() || Streamer::is_registered<T>() || std::is_trivial_v<T>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial");
		Streamer::read(value, *this);

		return *this;
	}

	template<typename TStream> 
	void ReadStream<TStream>::read(char* data, std::uint32_t size) {
		assert((m_read_ptr - Stream<TStream>::get_handle() + size) < Stream<TStream>::get_size());
		std::memcpy(data, m_read_ptr, size);
		m_read_ptr += size;
	}

	template<typename TStream> 
	template<typename T>
	void ReadStream<TStream>::advance_read_ptr() {
		std::uint32_t mod = reinterpret_cast<std::uintptr_t>(m_read_ptr) % TStream::template get_alignof<T>();
		m_read_ptr += mod;
		assert((m_read_ptr - Stream<TStream>::get_handle()) <= Stream<TStream>::get_size());
	}

	template<typename TStream>
		requires std::is_base_of_v<Stream<std::ios>, TStream> || std::is_same_v<std::ios, TStream>
	template<typename T>
	WriteStream<TStream>& WriteStream<TStream>::operator << (const T& value) {
		static_assert(Streamer::has_register<T>() || Streamer::is_registered<T>() || std::is_trivial_v<T>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial");
		Streamer::write(value, *this);

		return *this;
	}

	template<typename TStream>
		requires std::is_base_of_v<Stream<std::ios>, TStream> || std::is_same_v<std::ios, TStream>
	template<typename T>
	void WriteStream<TStream>::advance_write_ptr() {
		std::uint32_t mod = m_ostream.tellp() % StreamType::template get_alignof<T>();
		m_ostream.seekp(m_ostream.tellp() + mod);
	}

	template<typename TStream>
		requires std::is_base_of_v<Stream<std::ios>, TStream> || std::is_same_v<std::ios, TStream>
	void WriteStream<TStream>::write(const char* data, std::uint32_t size) {
		m_ostream.write(data, size);
	}

	template<typename TStream>
		requires std::is_base_of_v<Stream<std::ios>, TStream> || std::is_same_v<std::ios, TStream>
	template<typename T>
	ReadStream<TStream>& ReadStream<TStream>::operator >> (T& value) {
		static_assert(Streamer::has_register<T>() || Streamer::is_registered<T>() || std::is_trivial_v<T>, "T does not declare BEGIN() or END(), it is not globally serializable nor trivial");
		Streamer::read(value, *this);

		return *this;
	}

	template<typename TStream>
		requires std::is_base_of_v<Stream<std::ios>, TStream> || std::is_same_v<std::ios, TStream>
	void ReadStream<TStream>::read(char* data, std::uint32_t size) {
		m_istream.read(data, size);
	}
	
	template<typename TStream>
		requires std::is_base_of_v<Stream<std::ios>, TStream> || std::is_same_v<std::ios, TStream>
	template<typename T>
	void ReadStream<TStream>::advance_read_ptr() {
		std::uint32_t mod = m_istream.tellg() % StreamType::template get_alignof<T>();
		m_istream.seekg(m_istream.tellg() + mod);
	}
}

#endif	// EASER_STREAMER_H
