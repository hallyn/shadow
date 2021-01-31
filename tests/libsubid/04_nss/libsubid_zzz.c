#include <sys/types.h>
#include <stdbool.h>
#include <idmapping.h>
#include <subid.h>
#include <string.h>

bool has_any_range(const char *owner, enum subid_type t)
{
	if (t == ID_TYPE_UID)
		return strcmp(owner, "user1") == 0;
	return strcmp(owner, "group1") == 0;
}

bool has_range(const char *owner, unsigned long start, unsigned long count, enum subid_type t)
{
	if (t == ID_TYPE_UID)
		if (strcmp(owner, "user1") != 0)
			return false;
	if (t == ID_TYPE_GID)
		if (strcmp(owner, "group1") != 0)
			return false;
	if (start < 100000)
		return false;
	if (start +count > 165536)
		return false;
	return true;
}
