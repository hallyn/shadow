#include <sys/types.h>
#include <stdbool.h>

#ifndef SUBID_RANGE_DEFINED
#define SUBID_RANGE_DEFINED 1
struct subordinate_range {
	const char *owner;
	unsigned long start;
	unsigned long count;
};

enum subid_type {
	ID_TYPE_UID = 1,
	ID_TYPE_GID = 2
};

enum subid_status {
	SUBID_STATUS_SUCCESS = 0,
	SUBID_STATUS_UNKNOWN_USER = 1,
	SUBID_STATUS_ERROR_CONN = 2,
	SUBID_STATUS_ERROR = 3,
};

int get_subuid_ranges(const char *owner, struct subordinate_range ***ranges);
int get_subgid_ranges(const char *owner, struct subordinate_range ***ranges);
void subid_free_ranges(struct subordinate_range **ranges, int count);

int get_subuid_owners(uid_t uid, uid_t **owner);
int get_subgid_owners(gid_t gid, uid_t **owner);

/* range should be pre-allocated with owner and count filled in, start is
 * ignored, can be 0 */
bool grant_subuid_range(struct subordinate_range *range, bool reuse);
bool grant_subgid_range(struct subordinate_range *range, bool reuse);

bool ungrant_subuid_range(struct subordinate_range *range);
bool ungrant_subgid_range(struct subordinate_range *range);

#define SUBID_NFIELDS 3
#endif
