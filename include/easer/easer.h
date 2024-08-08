#ifndef EASER_EASER_H
#define EASER_EASER_H

#include "base.h"
#include <limits>
#include <fstream>
#include <functional>

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
	static consteval bool is_field_valid(::easer::Serializer::Field<__COUNTER__>) { \
		return true; \
	} \
	void serialize(std::ostream& stream, ::easer::Serializer::Field<__COUNTER__ - 1>) { \
		if constexpr (::easer::Serializer::is_serializable<field_type>()) { \
			::easer::Serializer::serialize<field_type>(field_name, stream); \
		} \
		else {\
			stream.write((const char*)&field_name, sizeof(field_name)); \
		}\
	} \
	void deserialize(std::istream& stream, ::easer::Serializer::Field<__COUNTER__ - 2>) { \
		if constexpr (::easer::Serializer::is_serializable<field_type>()) { \
			::easer::Serializer::deserialize<field_type>(field_name, stream); \
		} \
		else {\
			stream.read((char*)&field_name, sizeof(field_name)); \
		}\
	} \
	field_type field_name

#define END \
	static consteval unsigned int end() { \
		return __COUNTER__; \
	} \
	friend class Serializer; \
	template<typename T, typename V> \
	friend class has_end; \

namespace easer {
	class Serializer {
	public:
		template<unsigned int UFieldId>
		struct Field {};

	public:
		template<typename T>
		inline static void serialize(T& record, std::ostream& stream) {
			static_assert(is_serializable<T>() || std::is_fundamental_v<T>, "T does not declare BEGIN() or END() and it is not a primitive type");
			if constexpr (std::is_fundamental_v<T>) {
				stream.write((const char*)&record, sizeof(record));
			}
			else {
				std::invoke([&record, &stream]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<T, TArgs...>(), "BEGIN() specifies invalid bases that T does not inherit from");
					internal_dispatch_serialize<TArgs..., T>(&record, stream);

				}, static_cast<T::Inheritance::Bases*>(nullptr));
			}
		}

		template<typename T>
		inline static void deserialize(T& record, std::istream& stream) {
			static_assert(is_serializable<T>() || std::is_fundamental_v<T>, "T does not declare BEGIN() or END() and it is not a primitive type");
			if constexpr (std::is_fundamental_v<T>) {
				stream.read((char*)&record, sizeof(record));
			}
			else {
				std::invoke([&record, &stream]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<T, TArgs...>(), "BEGIN() specifies invalid bases that T does not inherit from");
					internal_dispatch_deserialize<TArgs..., T>(&record, stream);

				}, static_cast<T::Inheritance::Bases*>(nullptr));
			}
		}

		template<typename T>
		static consteval bool is_serializable() {
			return has_begin<T>::value || has_end<T>::value;
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
			if constexpr (sizeof...(TBases)) {
				if constexpr (std::is_base_of_v<TCurrent, TRecord>) {
					return is_inheritance_valid_recursive<TRecord, TBases...>();
				}
				else {
					return false;
				}
			}
			else {
				return true;
			}
		}

		template<typename TCurrent, typename... TBases>
		inline static void internal_dispatch_serialize(void* record, std::ostream& stream) {
			if constexpr (is_serializable<TCurrent>()) {
				std::invoke([&record, &stream]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						internal_dispatch_serialize<TArgs...>(record, stream);
					}

				}, static_cast<TCurrent::Inheritance::Bases*>(nullptr));

				internal_serialize<TCurrent, TCurrent::begin() + 1>(static_cast<TCurrent*>(record), stream);
			} 
			if constexpr (sizeof...(TBases)) {
				internal_dispatch_serialize<TBases...>(record, stream);
			}
		}

		template<typename T, unsigned int UFieldId>
		inline static void internal_serialize(T* record, std::ostream& stream) {
			const Field<UFieldId> field;
			if constexpr(T::end() == UFieldId) {
				return;
			}
			else if constexpr (T::is_field_valid(field)) {
				record->serialize(stream, field);
				internal_serialize<T, UFieldId + 1>(record, stream);
			}
			else {
				internal_serialize<T, UFieldId + 1>(record, stream);
			}
		}

		template<typename TCurrent, typename... TBases>
		inline static void internal_dispatch_deserialize(void* record, std::istream& stream) {
			if constexpr (is_serializable<TCurrent>()) {
				std::invoke([&record, &stream]<typename... TArgs>(const std::tuple<TArgs...>*) {
					static_assert(is_inheritance_valid<TCurrent, TArgs...>(), "BEGIN() specifies invalid bases that T does not inherit from");
					if constexpr (sizeof...(TArgs)) {
						internal_dispatch_deserialize<TArgs...>(record, stream);
					}

				}, static_cast<TCurrent::Inheritance::Bases*>(nullptr));

				internal_deserialize<TCurrent, TCurrent::begin() + 1>(static_cast<TCurrent*>(record), stream);
			} 
			if constexpr (sizeof...(TBases)) {
				internal_dispatch_deserialize<TBases...>(record, stream);
			}
		}

		template<typename T, unsigned int UFieldId>
		inline static void internal_deserialize(T* record, std::istream& stream) {
			const Field<UFieldId> field;
			if constexpr(T::end() == UFieldId) {
				return;
			}
			else if constexpr (T::is_field_valid(field)) {
				record->deserialize(stream, field);
				internal_deserialize<T, UFieldId + 1>(record, stream);
			}
			else {
				internal_deserialize<T, UFieldId + 1>(record, stream);
			}
		}
	};
}

#endif	// EASER_EASER_H
