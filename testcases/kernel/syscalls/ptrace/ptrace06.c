/*
 * check out-of-bound/unaligned addresses given to
 *  - {PEEK,POKE}{DATA,TEXT,USER}
 *  - {GET,SET}{,FG}REGS
 *  - {GET,SET}SIGINFO
 *
 * Copyright (c) 2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later
 */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <linux/ptrace.h>
#include <asm/ptrace.h>

#include "test.h"
#include "usctest.h"
#include "spawn_ptrace_child.c"

char *TCID = "ptrace06";

struct test_case_t {
	enum __ptrace_request request;
	long addr;
	long data;
} test_cases[] = {
	{ PTRACE_PEEKDATA, .addr = 0 },
	{ PTRACE_PEEKDATA, .addr = 1 },
	{ PTRACE_PEEKDATA, .addr = 2 },
	{ PTRACE_PEEKDATA, .addr = 3 },
	{ PTRACE_PEEKDATA, .addr = -1 },
	{ PTRACE_PEEKDATA, .addr = -2 },
	{ PTRACE_PEEKDATA, .addr = -3 },

	{ PTRACE_PEEKTEXT, .addr = 0 },
	{ PTRACE_PEEKTEXT, .addr = 1 },
	{ PTRACE_PEEKTEXT, .addr = 2 },
	{ PTRACE_PEEKTEXT, .addr = 3 },
	{ PTRACE_PEEKTEXT, .addr = -1 },
	{ PTRACE_PEEKTEXT, .addr = -2 },
	{ PTRACE_PEEKTEXT, .addr = -3 },
	{ PTRACE_PEEKTEXT, .addr = -1 },

	{ PTRACE_PEEKUSER, .addr = sizeof(struct pt_regs) * 3 + 0 },
	{ PTRACE_PEEKUSER, .addr = sizeof(struct pt_regs) * 3 + 1 },
	{ PTRACE_PEEKUSER, .addr = sizeof(struct pt_regs) * 3 + 2 },
	{ PTRACE_PEEKUSER, .addr = sizeof(struct pt_regs) * 3 + 3 },
	{ PTRACE_PEEKUSER, .addr = -1 },
	{ PTRACE_PEEKUSER, .addr = -2 },
	{ PTRACE_PEEKUSER, .addr = -3 },

	{ PTRACE_POKEDATA, .addr = 0 },
	{ PTRACE_POKEDATA, .addr = 1 },
	{ PTRACE_POKEDATA, .addr = 2 },
	{ PTRACE_POKEDATA, .addr = 3 },
	{ PTRACE_POKEDATA, .addr = -1 },
	{ PTRACE_POKEDATA, .addr = -2 },
	{ PTRACE_POKEDATA, .addr = -3 },

	{ PTRACE_POKETEXT, .addr = 0 },
	{ PTRACE_POKETEXT, .addr = 1 },
	{ PTRACE_POKETEXT, .addr = 2 },
	{ PTRACE_POKETEXT, .addr = 3 },
	{ PTRACE_POKETEXT, .addr = -1 },
	{ PTRACE_POKETEXT, .addr = -2 },
	{ PTRACE_POKETEXT, .addr = -3 },

	{ PTRACE_POKEUSER, .addr = sizeof(struct pt_regs) * 3 + 0 },
	{ PTRACE_POKEUSER, .addr = sizeof(struct pt_regs) * 3 + 1 },
	{ PTRACE_POKEUSER, .addr = sizeof(struct pt_regs) * 3 + 2 },
	{ PTRACE_POKEUSER, .addr = sizeof(struct pt_regs) * 3 + 3 },
	{ PTRACE_POKEUSER, .addr = -1 },
	{ PTRACE_POKEUSER, .addr = -2 },
	{ PTRACE_POKEUSER, .addr = -3 },

	{ PTRACE_GETREGS, .data = 0 },
	{ PTRACE_GETREGS, .data = 1 },
	{ PTRACE_GETREGS, .data = 2 },
	{ PTRACE_GETREGS, .data = 3 },
	{ PTRACE_GETREGS, .data = -1 },
	{ PTRACE_GETREGS, .data = -2 },
	{ PTRACE_GETREGS, .data = -3 },

#ifdef PTRACE_GETFGREGS
	{ PTRACE_GETFGREGS, .data = 0 },
	{ PTRACE_GETFGREGS, .data = 1 },
	{ PTRACE_GETFGREGS, .data = 2 },
	{ PTRACE_GETFGREGS, .data = 3 },
	{ PTRACE_GETFGREGS, .data = -1 },
	{ PTRACE_GETFGREGS, .data = -2 },
	{ PTRACE_GETFGREGS, .data = -3 },
#endif

	{ PTRACE_SETREGS, .data = 0 },
	{ PTRACE_SETREGS, .data = 1 },
	{ PTRACE_SETREGS, .data = 2 },
	{ PTRACE_SETREGS, .data = 3 },
	{ PTRACE_SETREGS, .data = -1 },
	{ PTRACE_SETREGS, .data = -2 },
	{ PTRACE_SETREGS, .data = -3 },

#ifdef PTRACE_SETFGREGS
	{ PTRACE_SETFGREGS, .data = 0 },
	{ PTRACE_SETFGREGS, .data = 1 },
	{ PTRACE_SETFGREGS, .data = 2 },
	{ PTRACE_SETFGREGS, .data = 3 },
	{ PTRACE_SETFGREGS, .data = -1 },
	{ PTRACE_SETFGREGS, .data = -2 },
	{ PTRACE_SETFGREGS, .data = -3 },
#endif

	{ PTRACE_GETSIGINFO, .data = 0 },
	{ PTRACE_GETSIGINFO, .data = 1 },
	{ PTRACE_GETSIGINFO, .data = 2 },
	{ PTRACE_GETSIGINFO, .data = 3 },
	{ PTRACE_GETSIGINFO, .data = -1 },
	{ PTRACE_GETSIGINFO, .data = -2 },
	{ PTRACE_GETSIGINFO, .data = -3 },

	{ PTRACE_SETSIGINFO, .data = 0 },
	{ PTRACE_SETSIGINFO, .data = 1 },
	{ PTRACE_SETSIGINFO, .data = 2 },
	{ PTRACE_SETSIGINFO, .data = 3 },
	{ PTRACE_SETSIGINFO, .data = -1 },
	{ PTRACE_SETSIGINFO, .data = -2 },
	{ PTRACE_SETSIGINFO, .data = -3 },
};

int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int argc, char *argv[])
{
	size_t i;
	long ret;
	int saved_errno;
	char *msg;

	if ((msg = parse_opts(argc, argv, NULL, NULL)))
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);

	make_a_baby(argc, argv);

	for (i = 0; i < ARRAY_SIZE(test_cases); ++i) {
		struct test_case_t *tc = &test_cases[i];

		errno = 0;
		ret = ptrace(tc->request, pid, (void *)tc->addr, (void *)tc->data);
		saved_errno = errno;
		if (ret != -1)
			tst_resm(TFAIL, "ptrace(%s, ..., %p, %p) returned %li instead of -1",
				strptrace(tc->request), tc->addr, tc->data, ret);
		else if (saved_errno != EIO && saved_errno != EFAULT)
			tst_resm(TFAIL, "ptrace(%s, ..., %p, %p) expected errno EIO or EFAULT; actual: %i (%s)",
				strptrace(tc->request), tc->addr, tc->data,
				saved_errno, strerror(saved_errno));
		else
			tst_resm(TPASS, "ptrace(%s, ..., %p, %p) failed as expected",
				strptrace(tc->request), tc->addr, tc->data);
	}

	/* hopefully this worked */
	ptrace(PTRACE_KILL, pid, NULL, NULL);

	tst_exit();
}