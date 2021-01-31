#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "../libsubid/subid.h"

#define NSSWITCH "/etc/nsswitch.conf"

// NSS plugin handling for subids
// If nsswitch has a line like
//    subid: sssd
// then sssd will be consulted for subids.  Unlike normal NSS dbs,
// only one db is supported at a time.  That's open to debate, but
// the subids are a pretty limited resource, and local files seem
// bound to step on any other allocations leading to insecure
// conditions.
bool nss_initialized;
void *subid_nss_handle;

void nss_exit() {
	if (nss_initialized && subid_nss_handle) {
		dlclose(subid_nss_handle);
		subid_nss_handle = NULL;
	}
}

// nsswitch_path is an argument only to support testing.
void nss_init(char *nsswitch_path) {
	FILE *nssfp = NULL;
	char *line = NULL, *p, *token;
	size_t len = 0;

	if (nss_initialized)
		return;

	if (!nsswitch_path)
		nsswitch_path = NSSWITCH;

	// read nsswitch.conf to check for a line like:
	//   subid:	files
	nssfp = fopen(nsswitch_path, "r");
	if (!nssfp) {
		fprintf(stderr, "Failed opening %s: %m", nsswitch_path);
		return;
	}
	while ((getline(&line, &len, nssfp)) != -1) {
		if (line[0] == '\0' || line[0] == '#')
			continue;
		if (strncasecmp(line, "subid:", 6) != 0)
			continue;
		p = &line[6];
		while ((*p) && isspace(*p))
			p++;
		for (token = strtok(p, " \n\t");
			token;
			token = strtok(NULL, " \n\t")) {
			char libname[65];
			if (strcmp(token, "files") == 0) {
				subid_nss_handle = NULL;
				goto done;
			}
			if (strlen(token) > 50) {
				fprintf(stderr, "Subid NSS module name too long: %s\n", token);
				fprintf(stderr, "Using files\n");
				goto done;
			}
			snprintf(libname, 64,  "libsubid_%s.so", token);
			subid_nss_handle = dlopen(libname, RTLD_LAZY);
			if (!subid_nss_handle) {
				fprintf(stderr, "Error opening %s: %s\n", libname, dlerror());
				fprintf(stderr, "Using files\n");
				goto done;
			}
			goto done;
		}
		fprintf(stderr, "No usable subid NSS module found, using files\n");
		goto done;
	}

done:
	nss_initialized = true;
	free(line);
	if (nssfp) {
		atexit(nss_exit);
		fclose(nssfp);
	}
}

bool nss_has_any_range(const char *owner, enum subid_type idtype)
{
	bool (*f)(const char *, enum subid_type);
	f = dlsym(subid_nss_handle, "has_any_range");
	if (NULL == f) {
		fprintf(stderr, "Error getting has_any_range function: %s", dlerror());
		return false;
	}
	return f(owner, idtype);
}

bool nss_has_range(const char *owner, unsigned long start, unsigned long count, enum subid_type idtype)
{
	bool (*f)(const char *, unsigned long, unsigned long, enum subid_type);
	f = dlsym(subid_nss_handle, "has_range");
	if (NULL == f) {
		fprintf(stderr, "Error getting has_range function: %s", dlerror());
		return false;
	}

	return f(owner, start, count, idtype);
}
