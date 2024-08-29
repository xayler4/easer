#ifndef EASER_BASE_H
#define EASER_BASE_H

#if defined _WIN32 || defined WIN32
	#define EASER_PLATFORM_WINDOWS
#elif defined __linux || defined __linux__ || defined linux
	#define EASER_PLATFORM_LINUX
#else
	#error Platform not supported
#endif

#include <cstdint>
#include <boost/preprocessor.hpp>

#define EXPAND_MEMBER(instance, member) instance.member
#define EXPAND_MEMBER_TYPE(type, member) decltype((*(static_cast<type*>(nullptr))).member)

#define EXPAND_MEMBER_HELPER(r, instance, member) \
	EXPAND_MEMBER(instance, member)

#define EXPAND_MEMBER_TYPE_HELPER(r, type, member) \
	EXPAND_MEMBER_TYPE(type, member)

#define EXPAND_MEMBER_HELPER(r, instance, member) \
	EXPAND_MEMBER(instance, member)

#define FIELD_NAME_VARIADIC_TO_FIELD_ACCESS_VARIADIC(instance, ...) \
	BOOST_PP_SEQ_ENUM( \
		BOOST_PP_SEQ_TRANSFORM(EXPAND_MEMBER_HELPER, instance, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	)

#define FIELD_NAME_VARIADIC_TO_FIELD_TYPE_VARIADIC(type, ...) \
	BOOST_PP_SEQ_ENUM( \
		BOOST_PP_SEQ_TRANSFORM(EXPAND_MEMBER_TYPE_HELPER, type, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	)

#endif	// EASER_BASE_H
