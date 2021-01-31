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

struct subid_nss_ops {
	/*
	 * nss_has_any_range: does a user own any subid range
	 *
	 * @owner: username
	 * @idtype: subuid or subgid
     * @result: true if a subid allocation was found for @owner
	 *
	 * returns success if the module was able to determine an answer (true or false),
     * else an error status.
	 */
	enum subid_status (*has_any_range)(const char *owner, enum subid_type idtype, bool *result);

	/*
	 * nss_has_range: does a user own a given subid range
	 *
	 * @owner: username
	 * @start: first subid in queried range
	 * @count: number of subids in queried range
	 * @idtype: subuid or subgid
     * @result: true if @owner has been allocated the subid range.
	 *
	 * returns success if the module was able to determine an answer (true or false),
     * else an error status.
	 */
	enum subid_status (*has_range)(const char *owner, unsigned long start, unsigned long count, enum subid_type idtype, bool *result);

	/*
	 * nss_list_owner_ranges: list the subid ranges delegated to a user.
	 *
	 * @owner - string representing username being queried
	 * @id_type - subuid or subgid
	 *
	 * Returns a NULL-terminated array of struct subordinate_range,
	 * or NULL.  The returned array of struct subordinate_range must be
	 * freed by the caller, if not NULL.
	 */
	struct subordinate_range **(*list_owner_ranges)(const char *owner, enum subid_type id_type);

	/*
	 * nss_find_subid_owners: find uids who own a given subuid or subgid.
	 *
	 * @owner - string representing username being queried
	 * @uids - pointer to an array of uids which will be allocated by
	 *         nss_find_subid_owners()
	 * @id_type - subuid or subgid
	 *
	 * Returns the number of uids which own the subid.  @uids must be freed by
	 * the caller.
	 */
	int (*find_subid_owners)(unsigned long id, uid_t **uids, enum subid_type id_type);

    /* The dlsym handle to close */
	void *handle;
};

#define SUBID_NFIELDS 3
#endif
