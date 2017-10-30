#define _GNU_SOURCE
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <link.h>
#include <dlfcn.h>

char *reloc_addr;

extern int B(int x);

int A() {
	printf("This is function A.\n");
	volatile int i = B(8);
}

__attribute__((no_instrument_function))
void __cyg_profile_func_enter(void *this, void *caller)
{
	void *buffer[100];
	char **strings;

	int nptrs = backtrace(buffer, 3);

	strings = backtrace_symbols(buffer, nptrs);
	if (strings == NULL) {
		perror("backtrace_symbols");
		exit(EXIT_FAILURE);
	}

	Dl_info i, j;

	memset(&i,0,sizeof(i));
	memset(&j,0,sizeof(i));
	if (dladdr(caller, &i) != 0);
	if (dladdr(this, &j) != 0);

	printf("instrument: %p <rel: %p> (=%s) called %p <rel: %p> (=%s)\n", caller,
		(char *)caller - reloc_addr,
		i.dli_sname,
		this,
		(char *)this - reloc_addr,
		j.dli_sname);

	if (nptrs == 3) {
		printf("backtrace: %s called %s\n", strings[2], strings[1]);
	}

	free(strings);
}

__attribute__((no_instrument_function))
static int callback(struct dl_phdr_info *info, size_t size, void *data)
{
	int j;
	for (j = 0; j < info->dlpi_phnum; j++) {
		if (info->dlpi_phdr[j].p_type == PT_LOAD) {
			const char *beg = (const char*) info->dlpi_addr + info->dlpi_phdr[j].p_vaddr;
			const char *end = beg + info->dlpi_phdr[j].p_memsz;
			const char *cb = (const char *)&callback;
			if (beg < cb && cb < end) {
				// Found PT_LOAD that "covers" callback().
				printf("ELF header is at %p, image linked at 0x%zx, relocation: 0x%zx\n",
					beg, info->dlpi_phdr[j].p_vaddr, info->dlpi_addr);
				reloc_addr = (char *)info->dlpi_addr;
				return 1;
			}
			return 0;
		}
	}
	return 0;
}

__attribute__((constructor,no_instrument_function))
void initprog() {
	dl_iterate_phdr(callback, NULL);
}

__attribute__((no_instrument_function))
int main(void)
{
	A();
	return 0;
}
