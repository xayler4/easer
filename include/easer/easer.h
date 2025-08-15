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
#include <optional>
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

#define ESR_REGISTER(channel_name, type, ...) \
		requires (::esr::Manager::is_in_set<ESR_API(TChannel), ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(channel_name)>() && std::is_same_v<type, std::remove_cvref_t<type>>) \
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
		inline static void write(const std::decay_t<type>& v, ::esr::WriteStream<TStream>& stream) { \
			stream.template advance_write_ptr<type>(); \
			internal_write(v, stream, ESR_FIELD_NAME_VARIADIC_TO_FIELD_ACCESS_VARIADIC(v, __VA_ARGS__)); \
		} \
		template<typename TStream> \
		inline static void read(std::decay_t<type>& v, ::esr::ReadStream<TStream>& stream) { \
			stream.template advance_read_ptr<type>(); \
			internal_read(v, stream, ESR_FIELD_NAME_VARIADIC_TO_FIELD_ACCESS_VARIADIC(v, __VA_ARGS__)); \
		} \
		template<typename TStream> \
		static consteval std::size_t get_sizeof() { \
			return internal_get_sizeof<ESR_FIELD_NAME_VARIADIC_TO_FIELD_TYPE_VARIADIC(type, __VA_ARGS__)>(); \
		} \
		template<typename TCurrent, typename TStream, typename... TArgs> \
		inline static void internal_write(const std::decay_t<type>& name, ::esr::WriteStream<TStream>& stream, TCurrent& write_ptr, TArgs&... args) { \
			stream.template advance_write_ptr<TCurrent>(); \
			::esr::Manager::write<TCurrent>(write_ptr, stream); \
			if constexpr(sizeof...(TArgs)) { \
				internal_write(name, stream, args...); \
			} \
		} \
		template<typename TCurrent, typename TStream, typename... TArgs> \
		inline static void internal_read(std::decay_t<type>& name, ::esr::ReadStream<TStream>& stream, TCurrent& write_ptr, TArgs&... args) { \
			::esr::Manager::read<TCurrent>(write_ptr, stream); \
			if constexpr(sizeof...(TArgs)) { \
				internal_read(name, stream, args...); \
			} \
		} \
		template<typename TCurrent, typename... TArgs> \
		static consteval std::size_t internal_get_sizeof() { \
			if constexpr(sizeof...(TArgs)) { \
				return sizeof(TCurrent) + internal_get_sizeof<TArgs...>(); \
			} \
			else { \
				return sizeof(TCurrent); \
			} \
		} \
		friend class ::esr::Manager; \
	}; \

#define ESR_REGISTER_NONE(channel_name, type) \
		requires (::esr::Manager::is_in_set<ESR_API(TChannel), ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(channel_name)>() && std::is_same_v<type, std::remove_cvref_t<type>>) \
	class ESR_API(Registry)<type, ESR_API(TChannel)> { \
	public: \
		static consteval bool is_valid() { \
			return true; \
		} \
		static consteval bool is_none() { \
			return true; \
		} \
		static consteval std::size_t get_sizeof() { \
			return 0; \
		} \
		friend class ::esr::Manager; \
	}; \

#define ESR_REGISTER_PROC_WRS(channel_name, type, type_name, stream_name, write_func_body, read_func_body, get_sizeof_func_body) \
		requires (::esr::Manager::is_in_set<ESR_API(TChannel), ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(channel_name)>() && std::is_same_v<type, std::remove_cvref_t<type>>) \
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
		inline static void write(const std::decay_t<type>& type_name, ::esr::WriteStream<ESR_API(TStream)>& stream_name) { \
			stream_name.template advance_write_ptr<type>(); \
			write_func_body \
		} \
		template<typename ESR_API(TStream)> \
		inline static void read(std::decay_t<type>& type_name, ::esr::ReadStream<ESR_API(TStream)>& stream_name) { \
			stream_name.template advance_read_ptr<type>(); \
			read_func_body \
		} \
		template<typename ESR_API(TStream)> \
		static consteval std::size_t get_sizeof() { \
			get_sizeof_func_body \
		} \
		friend class ::esr::Manager; \
	}; \

#define ESR_REGISTER_PROC_WR(channel_name, type, type_name, stream_name, write_func_body, read_func_body) \
		requires (::esr::Manager::is_in_set<ESR_API(TChannel), ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(channel_name)>() && std::is_same_v<type, std::remove_cvref_t<type>>) \
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
		inline static void write(const std::decay_t<type>& type_name, ::esr::WriteStream<ESR_API(TStream)>& stream_name) { \
			stream_name.template advance_write_ptr<type>(); \
			write_func_body \
		} \
		template<typename ESR_API(TStream)> \
		inline static void read(std::decay_t<type>& type_name, ::esr::ReadStream<ESR_API(TStream)>& stream_name) { \
			stream_name.template advance_read_ptr<type>(); \
			read_func_body \
		} \
		friend class ::esr::Manager; \
	}; \

#define ESR_REGISTER_PROC_WS(channel_name, type, type_name, stream_name, write_func_body, get_sizeof_func_body) \
		requires (::esr::Manager::is_in_set<ESR_API(TChannel), ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(channel_name)>() && std::is_same_v<type, std::remove_cvref_t<type>>) \
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
		inline static void write(const std::decay_t<type>& type_name, ::esr::WriteStream<ESR_API(TStream)>& stream_name) { \
			stream_name.template advance_write_ptr<type>(); \
			write_func_body \
		} \
		template<typename ESR_API(TStream)> \
		static consteval std::size_t get_sizeof() { \
			get_sizeof_func_body \
		} \
		friend class ::esr::Manager; \
	}; \

#define ESR_REGISTER_PROC_RS(channel_name, type, type_name, stream_name, read_func_body, get_sizeof_func_body) \
		requires (::esr::Manager::is_in_set<ESR_API(TChannel), ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(channel_name)>() && std::is_same_v<type, std::remove_cvref_t<type>>) \
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
		inline static void read(std::decay_t<type>& type_name, ::esr::ReadStream<ESR_API(TStream)>& stream_name) { \
			stream_name.template advance_read_ptr<type>(); \
			read_func_body \
		} \
		template<typename ESR_API(TStream)> \
		static consteval std::size_t get_sizeof() { \
			get_sizeof_func_body \
		} \
		friend class ::esr::Manager; \
	}; \

#define ESR_REGISTER_PROC_W(channel_name, type, type_name, stream_name, write_func_body) \
		requires (::esr::Manager::is_in_set<ESR_API(TChannel), ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(channel_name)>() && std::is_same_v<type, std::remove_cvref_t<type>>) \
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
		inline static void write(const std::decay_t<type>& type_name, ::esr::WriteStream<ESR_API(TStream)>& stream_name) { \
			stream_name.template advance_write_ptr<type>(); \
			write_func_body \
		} \
		friend class ::esr::Manager; \
	}; \

#define ESR_REGISTER_PROC_R(channel_name, type, type_name, stream_name, read_func_body) \
		requires (::esr::Manager::is_in_set<ESR_API(TChannel), ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(channel_name)>() && std::is_same_v<type, std::remove_cvref_t<type>>) \
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
		inline static void read(std::decay_t<type>& type_name, ::esr::ReadStream<ESR_API(TStream)>& stream_name) { \
			stream_name.template advance_read_ptr<type>(); \
			read_func_body \
		} \
		friend class ::esr::Manager; \
	}; \

#define ESR_REGISTER_PROC_S(channel_name, type, type_name, stream_name, get_sizeof_func_body) \
		requires (::esr::Manager::is_in_set<ESR_API(TChannel), ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(channel_name)>() && std::is_same_v<type, std::remove_cvref_t<type>>) \
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
		static consteval std::size_t get_sizeof() { \
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
		static_assert(::esr::Manager::has_register<field_type>() || ::esr::Manager::is_registered<field_type, TChannel>() || std::is_array_v<field_type> || ::esr::Manager::is_iterable<field_type>() || std::is_fundamental_v<field_type> || std::is_same_v<std::remove_cvref_t<field_type>, char*>, "T is not serializable"); \
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
	static consteval std::size_t ESR_API(get_sizeof)(const ::esr::Manager::Field<__COUNTER__ - 3>) { \
		return ::esr::Manager::get_sizeof<field_type>(); \
	} \
	field_type field_name

#define ESR_SPEC_FIELD(declaration_specifier, field_type, field_name, ...) \
	template<typename TChannel> \
	static consteval bool ESR_API(is_field_valid)(TChannel channel, const ::esr::Manager::Field<__COUNTER__>) { \
		static_assert(::esr::Manager::has_register<field_type>() || ::esr::Manager::is_registered<field_type, TChannel>() || std::is_array_v<field_type> || ::esr::Manager::is_iterable<field_type>() || std::is_fundamental_v<field_type> || std::is_same_v<std::remove_cvref_t<field_type>, char*>, "T is not serializable"); \
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
	static consteval std::size_t ESR_API(get_sizeof)(const ::esr::Manager::Field<__COUNTER__ - 3>) { \
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

	struct WriteStreamData {
		std::size_t size;
		std::optional<std::uint8_t*> data;
		std::optional<std::uint8_t*> write_ptr;
	};
 
	template<typename TStream>
	class Stream {
	public:
		template<typename T>
		static consteval std::size_t get_alignof() {
			return 1;
		}

		static consteval std::string_view get_channel() {
			return std::string_view{""};
		}

		static consteval bool should_trigger_write_too_large() {
			return false;
		}

		static consteval std::optional<std::size_t> get_post_write_stride() {
			return {};
		}

		static consteval std::optional<std::size_t> get_post_read_stride() {
			return {};
		}

		inline void on_post_write(std::size_t written_size) {
			assert(false);
		}

		inline WriteStreamData on_write_too_large(std::size_t required_size_delta) {
			assert(false);

			return {};
		}

		inline void set_data(std::uint8_t* data) {
			m_data = data;
		}

		inline void set_size(std::size_t size) {
			m_size = size;
		}

		inline void set_write_offset_since_post_write(std::size_t write_offset_since_post_write) {
			m_write_offset_since_post_write = write_offset_since_post_write;
		}

		inline void set_read_offset_since_post_read(std::size_t read_offset_since_post_read) {
			m_read_offset_since_post_read = read_offset_since_post_read;
		}

		inline const std::uint8_t* get_data() const {
			return m_data;
		}

		inline std::uint8_t* get_data() {
			return m_data;
		}

		inline std::size_t get_size() const {
			return m_size;
		}

		inline std::size_t get_read_offset_since_post_read() const {
			return m_read_offset_since_post_read;
		}

		inline std::size_t get_write_offset_since_post_write() const {
			return m_write_offset_since_post_write;
		}

	protected:
		Stream(std::uint8_t* data, std::size_t size) : 	
			m_data(data), 
			m_size(size),
			m_read_offset_since_post_read(0),
			m_write_offset_since_post_write(0)
		{

			static_assert(std::is_base_of_v<Stream<TStream>, TStream>);
		}

	private:
		std::uint8_t* m_data;
		std::size_t m_size;
		std::size_t m_write_offset_since_post_write;
		std::size_t m_read_offset_since_post_read;

		friend class esr::Manager;
	};

	template<>
	class Stream<std::ios> {
	public:
		template<typename T>
		static consteval std::size_t get_alignof() {
			return 1;
		}

		static consteval std::string_view get_channel() {
			return std::string_view{""};
		}

		static consteval bool should_trigger_write_too_large() {
			return false;
		}

		static consteval std::optional<std::size_t> get_post_write_stride() {
			return {};
		}

		static consteval std::optional<std::size_t> get_post_read_stride() {
			return {};
		}

	protected:
		Stream() = default;
	};

	template <typename TStream>
	concept StreamDerivedFromStandard = (std::is_base_of_v<Stream<std::ios>, TStream> && !std::is_same_v<std::ios, TStream>);

	template<typename TStream = std::ios>
	class WriteStream {
	public:
		using StreamType = TStream;

		WriteStream(StreamType& stream) : 
			m_stream(stream),
			m_write_idx(0) {

		}

		WriteStream(StreamType& stream, std::uint8_t* write_ptr) : 
			m_stream(stream) {

			set_write_ptr(write_ptr);
		}

		WriteStream(StreamType& stream, std::size_t write_idx) : 
			m_stream(stream),
			m_write_idx(write_idx) {
		}

		template<typename T>
		inline WriteStream<TStream>& operator << (const T& value);

		template<typename T>
		inline void advance_write_ptr();

		inline void write(const std::uint8_t* data, std::size_t size);
		
		inline void on_post_write(std::size_t written_size) {
			m_stream.on_post_write(written_size);
		}

		inline WriteStreamData on_write_too_large(std::size_t required_size_delta) {

			return m_stream.on_write_too_large(required_size_delta);
		}

		inline const std::uint8_t* get_data() const {
			return m_stream.get_data();
		}

		inline std::size_t get_size() const {
			return m_stream.get_size();
		}

		inline std::size_t get_write_offset_since_post_write() const {
			return m_stream.get_write_offset_since_post_write();
		}

		inline std::uint8_t* get_write_ptr() {
			return m_stream.get_data() + m_write_idx;
		}

		inline const std::uint8_t* get_write_ptr() const {
			return m_stream.get_data() + m_write_idx;
		}

		inline std::size_t get_write_idx() const {
			return m_write_idx;
		}

		inline void set_write_idx(std::size_t write_idx) {
			assert(write_idx < m_stream.get_size());

			m_write_idx = write_idx;
		}

		inline void set_write_ptr(std::uint8_t* write_ptr) {
			assert(write_ptr >= m_stream.get_data());

			m_write_idx = write_ptr - m_stream.get_data();
		}

		inline void set_data(std::uint8_t* data) {
			m_stream.set_data(data);
		}

		inline void set_size(std::size_t size) {
			m_stream.set_size(size);
		}

		inline void set_write_offset_since_post_write(std::size_t write_offset_since_post_write) {
			m_stream.set_write_offset_since_post_write(write_offset_since_post_write);
		}

	private:
		StreamType& m_stream;
		std::size_t m_write_idx;

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

		template<typename T>
		inline WriteStream<std::ios>& operator << (const T& value);

		inline bool is_eof() const{
			return m_ostream.eof();
		}

		template<typename T>
		inline void advance_write_ptr();

		inline void write(const std::uint8_t* data, std::size_t size);

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

		template<typename T>
		inline WriteStream<TStream>& operator << (const T& value);

		inline bool is_eof() const{
			return m_ostream.eof();
		}

		template<typename T>
		inline void advance_write_ptr();

		inline void write(const std::uint8_t* data, std::size_t size);

	private:
		std::ostream& m_ostream;

		friend class Manager;
	};

	template<typename TStream = std::ios>
	class ReadStream {
	public:
		using StreamType = TStream;

		ReadStream(StreamType& stream) : 
			m_stream(stream),
			m_read_idx(0) {

		}

		ReadStream(TStream& stream, std::uint8_t* read_ptr) : 
			m_stream(stream) {

			set_read_ptr(read_ptr);
		}

		template<typename T>
		inline ReadStream<TStream>& operator >> (T& value);

		template<typename T>
		inline void advance_read_ptr();

		inline void read(std::uint8_t* data, std::size_t size);

		inline void on_post_read(std::size_t read_size) {
			m_stream.on_post_read(read_size);
		}

		inline const std::uint8_t* get_data() const {
			return m_stream.get_data();
		}

		inline std::size_t get_size() const {
			return m_stream.get_size();
		}

		inline std::size_t get_read_offset_since_post_read() const {
			return m_stream.get_read_offset_since_post_read();
		}

		inline std::uint8_t* get_read_ptr() {
			return m_stream.get_data() + m_read_idx;
		}

		inline const std::uint8_t* get_read_ptr() const {
			return m_stream.get_data() + m_read_idx;
		}

		inline std::size_t get_read_idx() const {
			return m_read_idx;
		}

		inline void set_read_ptr(std::uint8_t* read_ptr) {
			assert(read_ptr >= m_stream.get_data());

			m_read_idx = read_ptr - m_stream.get_data();
		}

		inline void set_read_idx(std::size_t read_idx) {
			assert(read_idx < m_stream.get_size());

			m_read_idx = read_idx;
		}

		inline void set_data(std::uint8_t* data) {
			m_stream.set_data(data);
		}

		inline void set_size(std::size_t size) {
			m_stream.set_size(size);
		}

		inline void set_read_offset_since_post_read(std::size_t read_offset_since_post_read) {
			m_stream.set_read_offset_since_post_read(read_offset_since_post_read);
		}

	private:
		StreamType& m_stream;
		std::size_t m_read_idx;

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

		inline bool is_eof() const{
			return m_istream.eof();
		}

		template<typename T>
		inline void advance_read_ptr();

		inline void read(std::uint8_t* data, std::size_t size);

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

		inline bool is_eof() const{
			return m_istream.eof();
		}

		template<typename T>
		inline void advance_read_ptr();

		inline void read(std::uint8_t* data, std::size_t size);

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
			static_assert(has_register<T>() || is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<TStream>::StreamType::get_channel())>() || std::is_array_v<T> || ::esr::Manager::is_iterable<T>() || std::is_fundamental_v<T> || std::is_same_v<std::remove_cvref_t<T>, char*>, "T is not serializable");
			if constexpr(is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<TStream>::StreamType::get_channel())>()) {
				if constexpr (!::ESR_API(Registry)<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<TStream>::StreamType::get_channel())>::is_none()) {
					::ESR_API(Registry)<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<TStream>::StreamType::get_channel())>::write(record, stream);
				}
			}
			else if constexpr(has_register<T>()) {
				internal_dispatch_write<T, TStream, T>(record, stream);
			}
			else if constexpr(std::is_array_v<T>) {
				if constexpr (!is_registered<std::remove_cvref_t<std::remove_extent_t<T>>, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<TStream>::StreamType::get_channel())>() && std::is_fundamental_v<std::remove_extent_t<T>> && WriteStream<TStream>::StreamType::template get_alignof<std::remove_extent_t<T>>() == 1) {
					std::uint8_t* previous_write_ptr = stream.get_write_ptr();
					std::size_t record_size = sizeof(record);

					if constexpr (WriteStream<TStream>::StreamType::should_trigger_write_too_large() || WriteStream<TStream>::StreamType::get_post_write_stride().has_value()) {
						previous_write_ptr = stream.get_write_ptr();
					}
					
					handle_write_too_large<TStream>(stream, previous_write_ptr, record_size);
					previous_write_ptr = stream.get_write_ptr();
					stream.write((std::uint8_t*)&record, record_size);
					handle_post_write<TStream>(stream, previous_write_ptr, record_size);
				}
				else {
					for (std::size_t i = 0; i < std::extent_v<T>; i++) {
						write(record[i], stream);
					}
				}
			}
			else if constexpr(is_iterable<T>()) {
				if constexpr (std::contiguous_iterator<decltype(record.begin())> && !is_registered<std::remove_cvref_t<decltype(record.begin())>, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<TStream>::StreamType::get_channel())>() && std::is_fundamental_v<typename T::value_type> && WriteStream<TStream>::StreamType::template get_alignof<T>() == 1) {
					std::uint8_t* previous_write_ptr = nullptr;
					std::size_t record_size = sizeof(typename T::value_type) * (record.end() - record.begin());

					if constexpr (WriteStream<TStream>::StreamType::should_trigger_write_too_large() || WriteStream<TStream>::StreamType::get_post_write_stride().has_value()) {
						previous_write_ptr = stream.get_write_ptr();
					}

					handle_write_too_large<TStream>(stream, previous_write_ptr, record_size);
					previous_write_ptr = stream.get_write_ptr();
					stream.write((std::uint8_t*)&(*record.begin()), record_size);
					handle_post_write<TStream>(stream, previous_write_ptr, record_size);
				}
				else {
					for (auto& v : record) {
						write(v, stream);
					}
				}
			}
			else if constexpr(std::is_same_v<std::remove_cv_t<T>, char*>) {
				std::size_t i = 0;
				if (record != nullptr) {
					while (record[i] != '\n') {
						++i;	
					}

				}
				std::uint8_t* previous_write_ptr = nullptr;
				std::size_t record_size = i * sizeof(char);

				if constexpr (WriteStream<TStream>::StreamType::should_trigger_write_too_large() || WriteStream<TStream>::StreamType::get_post_write_stride().has_value()) {
					previous_write_ptr = stream.get_write_ptr();
				}

				handle_write_too_large<TStream>(stream, previous_write_ptr, record_size);
				previous_write_ptr = stream.get_write_ptr();
				stream.write((std::uint8_t*)&(*record.begin()), record_size);
				handle_post_write<TStream>(stream, previous_write_ptr, record_size);
			}
			else {
				std::uint8_t* previous_write_ptr = nullptr;
				std::size_t record_size = sizeof(record);

				if constexpr (WriteStream<TStream>::StreamType::should_trigger_write_too_large() || WriteStream<TStream>::StreamType::get_post_write_stride().has_value()) {
					previous_write_ptr = stream.get_write_ptr();
				}

				stream.template advance_write_ptr<T>();
				handle_write_too_large<TStream>(stream, previous_write_ptr, record_size);
				if constexpr (WriteStream<TStream>::StreamType::should_trigger_write_too_large() || WriteStream<TStream>::StreamType::get_post_write_stride().has_value()) {
					previous_write_ptr = stream.get_write_ptr();
				}
				stream.write((std::uint8_t*)&record, record_size);
				handle_post_write<TStream>(stream, previous_write_ptr, record_size);
			}
		}

		template<typename T, typename TStream>
		inline static void read(T& record, ReadStream<TStream>& stream) {
			static_assert(has_register<T>() || is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<TStream>::StreamType::get_channel())>() || std::is_array_v<T> || ::esr::Manager::is_iterable<T>() || std::is_fundamental_v<T>, "T is not serializable");
			if constexpr(is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<TStream>::StreamType::get_channel())>()) {
				if constexpr (!::ESR_API(Registry)<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<TStream>::StreamType::get_channel())>::is_none()) {
					::ESR_API(Registry)<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<TStream>::StreamType::get_channel())>::read(record, stream);
				 }
			}
			else if constexpr(has_register<T>()) {
				internal_dispatch_read<T, TStream, T>(record, stream);
			}
			else if constexpr(std::is_array_v<T>) {
				if constexpr (!is_registered<std::remove_cvref_t<std::remove_extent_t<T>>, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<TStream>::StreamType::get_channel())>() && std::is_fundamental_v<std::remove_extent_t<T>> && ReadStream<TStream>::StreamType::template get_alignof<std::remove_extent_t<T>>() == 1) {
					std::uint8_t* previous_read_ptr = nullptr;
					std::size_t record_size = sizeof(record);

					if constexpr (ReadStream<TStream>::StreamType::get_post_read_stride().has_value()) {
						previous_read_ptr = stream.get_read_ptr();
					}

					stream.read((std::uint8_t*)&record, record_size);
					handle_post_read<TStream>(stream, previous_read_ptr, record_size);
				}
				else {
					for (std::size_t i = 0; i < std::extent_v<T>; i++) {
						read(record[i], stream);
					}
				}
			}
			else if constexpr(is_iterable<T>()) {
				if constexpr (std::contiguous_iterator<decltype(record.begin())> && !is_registered<std::remove_cvref_t<decltype(record.begin())>, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<TStream>::StreamType::get_channel())>() && std::is_fundamental_v<typename T::value_type> && ReadStream<TStream>::StreamType::template get_alignof<T>() == 1) {
					std::uint8_t* previous_read_ptr = nullptr;
					std::size_t record_size = sizeof(typename T::value_type) * (record.end() - record.begin());
					if constexpr (ReadStream<TStream>::StreamType::get_post_read_stride().has_value()) {
						previous_read_ptr = stream.get_read_ptr();
					}

					stream.read((std::uint8_t*)&(*record.begin()), record_size);
					handle_post_read<TStream>(stream, previous_read_ptr, record_size);
				}
				else {
					for (auto& v : record) {
						read(v, stream);
					}
				}
			}
			else {
				std::uint8_t* previous_read_ptr = nullptr;
				std::size_t record_size = sizeof(record);

				if constexpr (ReadStream<TStream>::StreamType::get_post_read_stride().has_value()) {
					previous_read_ptr = stream.get_read_ptr();
				}

				stream.template advance_read_ptr<T>();
				stream.read((std::uint8_t*)&record, record_size);
				handle_post_read<TStream>(stream, previous_read_ptr, record_size);
			}
		}

		template<typename T, typename TStream = Stream<std::ios>>
		static consteval std::size_t get_sizeof() {
			static_assert(has_register<T>() || is_registered<T>() || std::is_array_v<T> || ::esr::Manager::is_iterable<T>() || std::is_fundamental_v<T>, "T is not serializable");

			if constexpr (is_registered<T>()) {
				return ::ESR_API(Registry)<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(TStream::get_channel())>::template get_sizeof<TStream>();
			}
			else if constexpr (has_register<T>()) {
				return internal_dispatch_get_sizeof<TStream, T>();
			}
			else if constexpr(std::is_array_v<T>) {
				return get_sizeof<std::remove_extent_t<T>, TStream>() * std::extent_v<T>;
			}
			else {
				return sizeof(T);
			}
		}

		template<typename T>
		static consteval bool has_register() {
			return has_begin<std::remove_cvref_t<T>>::value && has_end<std::remove_cvref_t<T>>::value;
		}

		template<typename T>
		static consteval bool is_iterable() {
			return has_begin_iterator<T>::value && has_end_iterator<T>::value;
		}

		template<typename T>
		static consteval bool is_registered() {
			return ::ESR_API(Registry)<std::remove_cvref_t<T>, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(std::string_view{""})>::is_valid();
		}

		template<typename T, typename TChannel>
		static consteval bool is_registered() {
			return ::ESR_API(Registry)<std::remove_cvref_t<T>, TChannel>::is_valid();
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

		template<typename, typename = std::void_t<>>
		struct has_begin_iterator : std::false_type{};

		template<typename T>
		struct has_begin_iterator<T, std::void_t<decltype(std::declval<T>().begin())>> : std::true_type{};

		template<typename, typename = std::void_t<>>
		struct has_end_iterator : std::false_type{};

		template<typename T>
		struct has_end_iterator<T, std::void_t<decltype(std::declval<T>().end())>> : std::true_type{};

	private:

		template<typename TStream>
		static void handle_write_too_large(WriteStream<TStream>& stream, std::uint8_t* previous_write_ptr, std::size_t record_size) {
			if constexpr (WriteStream<TStream>::StreamType::should_trigger_write_too_large()) {
				if (stream.get_write_idx() + record_size > stream.get_size()) {
					std::size_t delta = stream.get_write_ptr() - previous_write_ptr + record_size;
					WriteStreamData result = stream.on_write_too_large(delta);
					stream.set_size(result.size);
					if (result.data.has_value()) {
						stream.set_data(*result.data);
					}
					if (result.write_ptr.has_value()) {
						stream.set_write_ptr(*result.write_ptr);
					}
				}
			}
		}

		template<typename TStream>
		static void handle_post_write(WriteStream<TStream>& stream, std::uint8_t* previous_write_ptr, std::size_t record_size) {
			if constexpr (WriteStream<TStream>::StreamType::get_post_write_stride().has_value()) {
				std::size_t post_write_stride = *WriteStream<TStream>::StreamType::get_post_write_stride();
				std::size_t write_offset_since_post_write_delta = stream.get_write_ptr() - previous_write_ptr;
				std::size_t write_offset_since_post_write = stream.get_write_offset_since_post_write() + write_offset_since_post_write_delta;

				stream.set_write_offset_since_post_write(post_write_stride + write_offset_since_post_write);
				if (write_offset_since_post_write >= post_write_stride) {
					stream.on_post_write(write_offset_since_post_write);
					stream.set_write_offset_since_post_write(0);
				}

			}
		}

		template<typename TStream>
		static void handle_post_read(ReadStream<TStream>& stream, std::uint8_t* previous_read_ptr, std::size_t record_size) {
			if constexpr (ReadStream<TStream>::StreamType::get_post_read_stride().has_value()) {
				std::size_t post_read_stride = *ReadStream<TStream>::StreamType::get_post_read_stride();
				std::size_t read_offset_since_post_read_delta = stream.get_read_ptr() - previous_read_ptr;
				std::size_t read_offset_since_post_read = stream.get_read_offset_since_post_read() + read_offset_since_post_read_delta;

				stream.set_read_offset_since_post_read(post_read_stride + read_offset_since_post_read);
				if (read_offset_since_post_read >= post_read_stride) {
					stream.on_post_read(read_offset_since_post_read);
					stream.set_read_offset_since_post_read(0);
				}
			}
		}

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
			if constexpr (!has_register<TCurrent>()) {
				write(*static_cast<const TCurrent*>(&record), stream);
			}
			else {
				std::invoke([&record, &stream]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "ESR_BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						internal_dispatch_write<TRecord, TStream, TArgs...>(record, stream);
					}

				}, static_cast<TCurrent::ESR_API(Inheritance)::Bases*>(nullptr));

				stream.template advance_write_ptr<TCurrent>();
				internal_write<TCurrent, TStream, TCurrent::ESR_API(begin)() + 1>(*static_cast<const TCurrent*>(&record), stream);
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
				return internal_write<T, TStream, UFieldId + 1>(record, stream);
			}
			else {
				return internal_write<T, TStream, UFieldId + 1>(record, stream);
			}
		}

		template<typename TRecord, typename TStream, typename TCurrent, typename... TBases>
		inline static void internal_dispatch_read(TRecord& record, ReadStream<TStream>& stream) {
			if constexpr (!has_register<TCurrent>()) {
				read(*static_cast<TCurrent*>(&record), stream);
			}
			else {
				std::invoke([&record, &stream]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "ESR_BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						internal_dispatch_read<TRecord, TStream, TArgs...>(record, stream);
					}

				}, static_cast<TCurrent::ESR_API(Inheritance)::Bases*>(nullptr));

				stream.template advance_read_ptr<TCurrent>();
				internal_read<TCurrent, TStream, TCurrent::ESR_API(begin)() + 1>(*static_cast<TCurrent*>(&record), stream);
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
				return internal_read<T, TStream, UFieldId + 1>(record, stream);
			}
			else {
				return internal_read<T, TStream, UFieldId + 1>(record, stream);
			}
		}

		template<typename TStream, typename TCurrent, typename... TBases>
		static consteval std::size_t internal_dispatch_get_sizeof() {
			if constexpr (!has_register<TCurrent>()) {
				if constexpr (sizeof...(TBases)) {
					return get_sizeof<TCurrent>() + internal_dispatch_get_sizeof<TStream, TBases...>();
				}
				else {
					return get_sizeof<TCurrent>();
				}
			}
			else {
				const std::size_t partial_sizeof = std::invoke([]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "ESR_BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						return internal_dispatch_get_sizeof<TStream, TArgs...>();
					}
					else {
						return 0;
					}
				}, static_cast<TCurrent::ESR_API(Inheritance)::Bases*>(nullptr));

				const std::size_t return_sizeof = partial_sizeof + internal_get_sizeof<TStream, TCurrent, TCurrent::ESR_API(begin)() + 1>();

				if constexpr (sizeof... (TBases)) {
					return return_sizeof + internal_dispatch_get_sizeof<TStream, TBases...>();
				}
				else {
					return return_sizeof;
				}
			}
		}

		template<typename TStream, typename T, std::size_t UFieldId>
		static consteval std::size_t internal_get_sizeof() {
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
		static_assert(Manager::has_register<T>() || Manager::is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<TStream>::StreamType::get_channel())>() || std::is_array_v<T> || ::esr::Manager::is_iterable<T>() || std::is_fundamental_v<T> || std::is_same_v<std::remove_cvref_t<T>, char*>, "T is not serializable");
		Manager::write(value, *this);

		return *this;
	}

	template<typename TStream> 
	template<typename T>
	void WriteStream<TStream>::advance_write_ptr() {
		if constexpr (StreamType::template get_alignof<T>() > 1) {
			std::size_t mod = reinterpret_cast<std::uintptr_t>(get_write_ptr()) % TStream::template get_alignof<T>();
			m_write_idx += mod;
			assert(m_write_idx < m_stream.get_size());
		}
	}

	template<typename TStream> 
	void WriteStream<TStream>::write(const std::uint8_t* data, std::size_t size) {
		assert(m_write_idx + size <= m_stream.get_size());
		std::memcpy(get_write_ptr(), data, size);
		m_write_idx += size;
	}

	template<typename T>
	WriteStream<std::ios>& WriteStream<std::ios>::operator << (const T& value) {
		static_assert(Manager::has_register<T>() || Manager::is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<std::ios>::StreamType::get_channel())>() || std::is_array_v<T> || ::esr::Manager::is_iterable<T>() || std::is_fundamental_v<T> || std::is_same_v<std::remove_cvref_t<T>, char*>, "T is not serializable");
		Manager::write(value, *this);

		return *this;
	}

	template<typename T>
	void WriteStream<std::ios>::advance_write_ptr() {
		if constexpr (StreamType::template get_alignof<T>() > 1) {
			std::size_t mod = static_cast<std::size_t>(m_ostream.tellp()) % StreamType::template get_alignof<T>();
			m_ostream.seekp(static_cast<std::size_t>(m_ostream.tellp()) + mod);
		}
	}

	void WriteStream<std::ios>::write(const std::uint8_t* data, std::size_t size) {
		m_ostream.write(reinterpret_cast<const char*>(data), size);
	}

	template<StreamDerivedFromStandard TStream>
	template<typename T>
	WriteStream<TStream>& WriteStream<TStream>::operator << (const T& value) {
		static_assert(Manager::has_register<T>() || Manager::is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(WriteStream<TStream>::StreamType::get_channel())>() || std::is_array_v<T> || ::esr::Manager::is_iterable<T>() || std::is_fundamental_v<T> || std::is_same_v<std::remove_cvref_t<T>, char*>, "T is not serializable");
		Manager::write(value, *this);

		return *this;
	}

	template<StreamDerivedFromStandard TStream>
	template<typename T>
	void WriteStream<TStream>::advance_write_ptr() {
		if constexpr (StreamType::template get_alignof<T>() > 1) {
			std::size_t mod = static_cast<std::size_t>(m_ostream.tellp()) % StreamType::template get_alignof<T>();
			m_ostream.seekp(static_cast<std::size_t>(m_ostream.tellp()) + mod);
		}
	}

	template<StreamDerivedFromStandard TStream>
	void WriteStream<TStream>::write(const std::uint8_t* data, std::size_t size) {
		m_ostream.write(reinterpret_cast<const char*>(data), size);
	}

	template<typename TStream> 
	template<typename T>
	ReadStream<TStream>& ReadStream<TStream>::operator >> (T& value) {
		static_assert(Manager::has_register<T>() || Manager::is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<TStream>::StreamType::get_channel())>() || std::is_array_v<T> || ::esr::Manager::is_iterable<T>() || std::is_fundamental_v<T>, "T is not serializable");
		Manager::read(value, *this);

		return *this;
	}

	template<typename TStream> 
	template<typename T>
	void ReadStream<TStream>::advance_read_ptr() {
		if constexpr (StreamType::template get_alignof<T>() > 1) {
			std::size_t mod = reinterpret_cast<std::uintptr_t>(get_read_ptr()) % TStream::template get_alignof<T>();
			m_read_idx += mod;
			assert(m_read_idx < m_stream.get_size());
		}
	}

	template<typename TStream> 
	void ReadStream<TStream>::read(std::uint8_t* data, std::size_t size) {
		assert(m_read_idx + size <= get_size());
		std::memcpy(data, get_read_ptr(), size);
		m_read_idx += size;
	}

	template<typename T>
	ReadStream<std::ios>& ReadStream<std::ios>::operator >> (T& value) {
		static_assert(Manager::has_register<T>() || Manager::is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<std::ios>::StreamType::get_channel())>() || std::is_array_v<T> || ::esr::Manager::is_iterable<T>() || std::is_fundamental_v<T>, "T is not serializable");
		Manager::read(value, *this);

		return *this;
	}

	template<typename T>
	void ReadStream<std::ios>::advance_read_ptr() {
		if constexpr (StreamType::template get_alignof<T>() > 1) {
			std::size_t mod = static_cast<std::size_t>(m_istream.tellg()) % StreamType::template get_alignof<T>();
			m_istream.seekg(static_cast<std::size_t>(m_istream.tellg()) + mod);
		}
	}

	void ReadStream<std::ios>::read(std::uint8_t* data, std::size_t size) {
		m_istream.read(reinterpret_cast<char*>(data), size);
	}

	template<StreamDerivedFromStandard TStream>
	template<typename T>
	ReadStream<TStream>& ReadStream<TStream>::operator >> (T& value) {
		static_assert(Manager::has_register<T>() || Manager::is_registered<T, ESR_CHANNEL_NAME_VARIADIC_TO_CHANNEL_LAMBDA_TYPE_VARIADIC(ReadStream<TStream>::StreamType::get_channel())>() || std::is_array_v<T> || ::esr::Manager::is_iterable<T>() || std::is_fundamental_v<T>, "T is not serializable");
		Manager::read(value, *this);

		return *this;
	}

	template<StreamDerivedFromStandard TStream>
	template<typename T>
	void ReadStream<TStream>::advance_read_ptr() {
		if constexpr (StreamType::template get_alignof<T>() > 1) {
			std::size_t mod = static_cast<std::size_t>(m_istream.tellg()) % StreamType::template get_alignof<T>();
			m_istream.seekg(static_cast<std::size_t>(m_istream.tellg()) + mod);
		}
	}

	template<StreamDerivedFromStandard TStream>
	void ReadStream<TStream>::read(std::uint8_t* data, std::size_t size) {
		m_istream.read(reinterpret_cast<char*>(data), size);
	}
}

#endif	// EASER_EASER_H
