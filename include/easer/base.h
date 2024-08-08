#ifndef EASER_BASE_H
#define EASER_BASE_H

#if defined _WIN32 || defined WIN32
	#define EASER_PLATFORM_WINDOWS
#elif defined __linux || defined __linux__ || defined linux
	#define EASER_PLATFORM_LINUX
#else
	#error Platform not supported
#endif

#endif	// EASER_BASE_H
