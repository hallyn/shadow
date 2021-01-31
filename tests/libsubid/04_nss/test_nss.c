#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <prototypes.h>
#include <stdbool.h>
#include <dlfcn.h>

extern bool nss_initialized;
extern void *subid_nss_handle;

int main(int argc, char *argv[])
{
	int count=1;

	printf("starting nss parsing tests\n");
	setenv("LD_LIBRARY_PATH", ".", 1);
	// nsswitch2 has no subid: entry
	printf("test %d (no subid entry)\n", count++);
	nss_init("./nsswitch1.conf");
	if (!nss_initialized || subid_nss_handle)
		exit(1);
	// second run should change nothing
	printf("test %d (no subid entry, second run)\n", count++);
	nss_init("./nsswitch1.conf");
	if (!nss_initialized || subid_nss_handle)
		exit(1);
	nss_initialized = false;
	// nsswitch2 has a subid: files entry
	printf("test %d ('files' subid entry)\n", count++);
	nss_init("./nsswitch2.conf");
	if (!nss_initialized || subid_nss_handle)
		exit(1);
	// second run should change nothing
	printf("test %d ('files' subid entry, second run)\n", count++);
	nss_init("./nsswitch2.conf");
	if (!nss_initialized || subid_nss_handle)
		exit(1);
	// nsswitch3 has a subid: testnss entry
	printf("test %d ('test' subid entry)\n", count++);
	nss_initialized = false;
	nss_init("./nsswitch3.conf");
	if (!nss_initialized || !subid_nss_handle)
		exit(1);
	// second run should change nothing
	printf("test %d ('test' subid entry, second run)\n", count++);
	nss_init("./nsswitch3.conf");
	if (!nss_initialized || !subid_nss_handle)
		exit(1);
	printf("nss parsing tests done\n");
	exit(0);
}
