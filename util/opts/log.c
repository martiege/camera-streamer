#include "util/opts/log.h"
#include "util/opts/opts.h"

#include <string.h>

char *
strstrn(const char *s, const char *find, size_t len)
{
	char c, sc;
	if ((c = *find++) != 0) {
		len--;

		do {
			do {
				if ((sc = *s++) == 0)
					return (NULL);
			} while (sc != c);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

bool filter_log(const char *filename)
{
  if (!log_options.filter[0])
    return false;

  const char *ptr = log_options.filter;
  do {
    const char *next = strchr(ptr, OPTION_VALUE_LIST_SEP[0]);
    if (!next)
      next = ptr + strlen(ptr);

    if(strstrn(filename, ptr, next - ptr))
      return true;

		ptr = next;
  } while(*ptr++);

  return false;
}

int shrink_to_block(int size, int block)
{
	return size / block * block;
}

uint64_t get_time_us(clockid_t clock, struct timespec *ts, struct timeval *tv, int64_t delays_us)
{
	struct timespec now;

	if (clock != CLOCK_FROM_PARAMS) {
		clock_gettime(clock, &now);
	} else if (ts) {
		now = *ts;
	} else if (tv) {
		now.tv_sec = tv->tv_sec;
		now.tv_nsec = tv->tv_usec * 1000;
	} else {
		return -1;
	}

	if (delays_us > 0) {
		#define NS_IN_S (1000LL * 1000LL * 1000LL)
		int64_t nsec = now.tv_nsec + delays_us * 1000LL;
		now.tv_nsec = nsec % NS_IN_S;
		now.tv_sec += nsec / NS_IN_S;
	}

	if (ts) {
		*ts = now;
	}
	if (tv) {
		tv->tv_sec = now.tv_sec;
		tv->tv_usec = now.tv_nsec / 1000;
	}
	return now.tv_sec * 1000000LL + now.tv_nsec / 1000;
}

uint64_t get_monotonic_time_us(struct timespec *ts, struct timeval *tv)
{
	return get_time_us(CLOCK_MONOTONIC, ts, tv, 0);
}

int ioctl_retried(const char *name, int fd, int request, void *arg)
{
#define MAX_RETRIES 4

	int retries = 4;
	int ret = -1;

	while (retries-- > 0) {
		ret = ioctl(fd, request, arg);
		if (errno != EINTR && errno != EAGAIN && errno != ETIMEDOUT)
			break;
	}

	if (ret && retries <= 0) {
		LOG_PERROR(NULL, "%s: ioctl(%08x) retried %u times; giving up", name, request, MAX_RETRIES);
	}
	return ret;
}
