#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdatomic.h>
#include "prototypes.h"
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
static atomic_flag nss_init_started;
bool nss_init_completed;

static struct subid_nss_ops *subid_nss;

bool nss_is_initialized() {
	return nss_init_completed;
}

void nss_exit() {
	if (nss_is_initialized() && subid_nss) {
		dlclose(subid_nss->handle);
		free(subid_nss);
		subid_nss = NULL;
	}
}

// nsswitch_path is an argument only to support testing.
void nss_init(char *nsswitch_path) {
	FILE *nssfp = NULL;
	char *line = NULL, *p, *token, *saveptr;
	size_t len = 0;

	if (atomic_flag_test_and_set(&nss_init_started)) {
		// Another thread has started nss_init, wait for it to complete
		while (!nss_init_completed)
			usleep(100);
		return;
	}

	if (!nsswitch_path)
		nsswitch_path = NSSWITCH;

	// read nsswitch.conf to check for a line like:
	//   subid:	files
	nssfp = fopen(nsswitch_path, "r");
	if (!nssfp) {
		fprintf(stderr, "Failed opening %s: %m", nsswitch_path);
		nss_init_completed = true; // let's not keep trying
		return;
	}
	while ((getline(&line, &len, nssfp)) != -1) {
		if (line[0] == '\0' || line[0] == '#')
			continue;
		if (strlen(line) < 8)
			continue;
		if (strncasecmp(line, "subid:", 6) != 0)
			continue;
		p = &line[6];
		while ((*p) && isspace(*p))
			p++;
		if (!*p)
			continue;
		for (token = strtok_r(p, " \n\t", &saveptr);
		     token;
		     token = strtok_r(NULL, " \n\t", &saveptr)) {
			char libname[65];
			void *h;
			if (strcmp(token, "files") == 0) {
				subid_nss = NULL;
				goto done;
			}
			if (strlen(token) > 50) {
				fprintf(stderr, "Subid NSS module name too long (longer than 50 characters): %s\n", token);
				fprintf(stderr, "Using files\n");
				subid_nss = NULL;
				goto done;
			}
			snprintf(libname, 64,  "libsubid_%s.so", token);
			h = dlopen(libname, RTLD_LAZY);
			if (!h) {
				fprintf(stderr, "Error opening %s: %s\n", libname, dlerror());
				fprintf(stderr, "Using files\n");
				subid_nss = NULL;
				goto done;
			}
			subid_nss = malloc(sizeof(*subid_nss));
			if (!subid_nss) {
				dlclose(h);
				goto done;
			}
			subid_nss->has_range = dlsym(h, "shadow_subid_has_range");
			if (!subid_nss->has_range) {
				fprintf(stderr, "%s did not provide @has_range@\n", libname);
				dlclose(h);
				free(subid_nss);
				subid_nss = NULL;
				goto done;
			}
			subid_nss->list_owner_ranges = dlsym(h, "shadow_subid_list_owner_ranges");
			if (!subid_nss->list_owner_ranges) {
				fprintf(stderr, "%s did not provide @list_owner_ranges@\n", libname);
				dlclose(h);
				free(subid_nss);
				subid_nss = NULL;
				goto done;
			}
			subid_nss->has_any_range = dlsym(h, "shadow_subid_has_any_range");
			if (!subid_nss->has_any_range) {
				fprintf(stderr, "%s did not provide @has_any_range@\n", libname);
				dlclose(h);
				free(subid_nss);
				subid_nss = NULL;
				goto done;
			}
			subid_nss->find_subid_owners = dlsym(h, "shadow_subid_find_subid_owners");
			if (!subid_nss->find_subid_owners) {
				fprintf(stderr, "%s did not provide @find_subid_owners@\n", libname);
				dlclose(h);
				free(subid_nss);
				subid_nss = NULL;
				goto done;
			}
			subid_nss->handle = h;
			goto done;
		}
		fprintf(stderr, "No usable subid NSS module found, using files\n");
		// subid_nss has to be null here, but to ease reviews:
		free(subid_nss);
		subid_nss = NULL;
		goto done;
	}

done:
	nss_init_completed = true;
	free(line);
	if (nssfp) {
		atexit(nss_exit);
		fclose(nssfp);
	}
}

struct subid_nss_ops *get_subid_nss_handle() {
	nss_init(NULL);
	return subid_nss;
}
