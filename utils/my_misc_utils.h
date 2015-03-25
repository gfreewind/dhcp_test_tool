#pragma once

#define OFFSET_MEMBER(type, member)					\
		(((type*)0)->member)

#define bzero(dst, size)    memset(dst, 0, size);
