/*
 * $Header$
 *
 * Handles watchdog connection, and protocol communication with pgpool-II
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
<<<<<<< HEAD
 * Copyright (c) 2003-2012	PgPool Global Development Group
=======
 * Copyright (c) 2003-2011	PgPool Global Development Group
>>>>>>> 5fb22fa044700b511711f8808d2988c2147ff331
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of the
 * author not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. The author makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "pool.h"
#include "pool_config.h"
#include "watchdog.h"
#include "wd_ext.h"

int wd_init(void);

int
wd_init(void)
{
	struct timeval tv;

	/* set startup time */
	gettimeofday(&tv, NULL);

	/* allocate watchdog list */
	if (WD_List == NULL)
	{
		WD_List = pool_shared_memory_create(sizeof(WdInfo) * MAX_WATCHDOG_NUM);
		if (WD_List == NULL)
		{
			pool_error("failed to allocate watchdog list");
			return WD_NG;
		}
		memset(WD_List, 0, sizeof(WdInfo) * MAX_WATCHDOG_NUM);
	}
	/* set myself to watchdog list */
	wd_set_wd_list(pool_config->pgpool2_hostname, pool_config->port, pool_config->wd_port, &tv, WD_NORMAL);
	/* set other pgpools to watchdog list */
	wd_add_wd_list(pool_config->other_wd);

	/* check upper connection */
	if (wd_is_upper_ok(pool_config->trusted_servers) != WD_OK)
	{
		pool_error("failed to connect trusted server");
		return WD_NG;
	}

	/* send startup packet */
	wd_startup();

	/* check existence of master pgpool */
	if (wd_is_exist_master() == NULL )
	{
		/* escalate to delegate_IP holder */
		wd_escalation();
	}

	return WD_OK;	
}
