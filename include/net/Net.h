#pragma once

#include <winsock2.h>
#include <cstdio>

namespace ducklib
{
void net_initialize();
void net_shutdown();

template <typename... Args>
void net_log_error(const char* text, Args... args);

#define DL_NET_LOG
#define DL_NET_LOG_ERROR(text, ...) (printf(text, __VA_ARGS__))

#define DL_NET_FAIL(text, ...)					\
	do											\
	{											\
		DL_NET_LOG_ERROR(text, __VA_ARGS__);	\
		exit(0);								\
	} while (false)
}
