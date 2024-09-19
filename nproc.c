#include <stdio.h>
#include <stdlib.h>
#include <sys/sysctl.h>
#include <stdint.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <sys/cdefs.h>

#undef asizeof
#define asizeof(x)	(sizeof(x) / sizeof(*x))

static int hw_sysctl_get_v(int key)
{
	int mib[2], ret;
	size_t len;

	mib[0] = CTL_HW;
	mib[1] = key;
	len = sizeof(ret);

	if (sysctl(mib, asizeof(mib), &ret, &len, NULL, 0) == -1)
		err(EXIT_FAILURE, "sysctl()");

	return (ret);
}

static int to_int(const char *s)
{
	char *eptr;
	long ret;

	ret = strtol(s, &eptr, 10);
	if (errno != 0)
		err(EXIT_FAILURE, "strtol()");

	if (eptr == s)
		errx(EXIT_FAILURE, "no digits are found in '%s'.", s);

	/* Make sure not to overflow. */
	if (ret > INT32_MAX)
		errx(EXIT_FAILURE, "input value cannot be larger than INT32_MAX.");
	/* We can't accept negative values. */
	if (ret < 0)
		errx(EXIT_FAILURE, "resulted value cannot be smaller than 0.");

	return ((int)ret);
}

__dead
static void print_usage(int status)
{
	FILE *out;

	out = (status == EXIT_SUCCESS) ? stdout : stderr;

	fprintf(out, "usage: %s [-aofeh]\n", getprogname());
	exit(status);
}

int main(int argc, char **argv)
{
	int c, eflag, ncores, key;

        eflag = ncores = 0;	
	key = HW_NCPUFOUND;

	while ((c = getopt(argc, argv, "aofeh")) != -1) {
		switch (c) {
		case 'a':
		        key = HW_NCPU;
			break;
		case 'o':
		        key = HW_NCPUONLINE;
			break;
		case 'f':
			key = HW_NCPUFOUND;
			break;
		case 'e':
			eflag = 1;
			break;
		case 'h':
			print_usage(EXIT_SUCCESS);
		case '?':
		default:
			print_usage(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;

	ncores = hw_sysctl_get_v(key);
	if (eflag) {
		if (argv[0] == NULL)
			errx(EXIT_FAILURE, "no argument was provided.");
		ncores -= to_int(argv[0]);
		if (ncores <= 0)
			ncores = 1;
        }

	fprintf(stdout, "%d\n", ncores);
}
