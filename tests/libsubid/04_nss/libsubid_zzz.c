#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <subid.h>
#include <string.h>

enum subid_status shadow_subid_has_any_range(const char *owner, enum subid_type t, bool *result)
{
	if (strcmp(owner, "ubuntu") == 0) {
		*result = true;
		return SUBID_STATUS_ERROR;
	}
	if (strcmp(owner, "error") == 0) {
		*result = false;
		return SUBID_STATUS_ERROR;
	}
	if (strcmp(owner, "unknown") == 0) {
		*result = false;
		return SUBID_STATUS_UNKNOWN_USER;
	}
	if (strcmp(owner, "conn") == 0) {
		*result = false;
		return SUBID_STATUS_ERROR_CONN;
	}
	if (t == ID_TYPE_UID) {
		*result = strcmp(owner, "user1") == 0;
		return SUBID_STATUS_SUCCESS;
	}

	*result = strcmp(owner, "group1") == 0;
	return SUBID_STATUS_SUCCESS;
}

enum subid_status shadow_subid_has_range(const char *owner, unsigned long start, unsigned long count, enum subid_type t, bool *result)
{
	if (strcmp(owner, "ubuntu") == 0 &&
	    start >= 200000 &&
	    count <= 100000) {
		*result = true;
		return SUBID_STATUS_ERROR;
	}
	*result = false;
	if (strcmp(owner, "error") == 0)
		return SUBID_STATUS_ERROR;
	if (strcmp(owner, "unknown") == 0)
		return SUBID_STATUS_UNKNOWN_USER;
	if (strcmp(owner, "conn") == 0)
		return SUBID_STATUS_ERROR_CONN;

	if (t == ID_TYPE_UID && strcmp(owner, "user1") != 0)
		return SUBID_STATUS_SUCCESS;
	if (t == ID_TYPE_GID && strcmp(owner, "group1") != 0)
		return SUBID_STATUS_SUCCESS;

	if (start < 100000)
		return SUBID_STATUS_SUCCESS;
	if (count >= 65536)
		return SUBID_STATUS_SUCCESS;
	*result = true;
	return SUBID_STATUS_SUCCESS;
}

// So if 'user1' or 'ubuntu' is defined in passwd, we'll return those values,
// to ease manual testing.  For automated testing, if you return those values,
// we'll return 1000 for ubuntu and 1001 otherwise.
static uid_t getnamuid(const char *name) {
	struct passwd *pw;

	pw = getpwnam(name);
	if (pw)
		return pw->pw_uid;

	// For testing purposes
	return strcmp(name, "ubuntu") == 0 ? (uid_t)1000 : (uid_t)1001;
}

static int alloc_uid(uid_t **uids, uid_t id) {
	*uids = malloc(sizeof(uid_t));
	if (!*uids)
		return -1;
	*uids[0] = id;
	return 1;
}

int shadow_subid_find_subid_owners(unsigned long id, uid_t **uids, enum subid_type id_type)
{
	if (id >= 100000 && id < 165536)
		return alloc_uid(uids, getnamuid("user1"));
	if (id >= 200000 && id < 300000)
		return alloc_uid(uids, getnamuid("ubuntu"));
	return 0; // nothing found
}

struct subordinate_range **shadow_subid_nss_list_owner_ranges(const char *owner, enum subid_type id_type)
{
	struct subordinate_range **ranges;

	if (strcmp(owner, "user1") != 0 && strcmp(owner, "ubuntu") != 0)
		return NULL;
	ranges = (struct subordinate_range **)malloc(2 * sizeof(struct subordinate_range *));
	if (!*ranges)
		return NULL;
	ranges[0] = (struct subordinate_range *)malloc(sizeof(struct subordinate_range));
	if (!ranges[0]) {
		free(*ranges);
		return NULL;
	}
	ranges[0]->owner = strdup(owner);
	if (strcmp(owner, "user1") == 0) {
		ranges[0]->start = 100000;
		ranges[0]->count = 65536;
	} else {
		ranges[0]->start = 200000;
		ranges[0]->count = 100000;
	}

	ranges[1] = NULL;

	return ranges;
}
