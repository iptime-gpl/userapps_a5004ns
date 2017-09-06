/*
 * $Id: ppd.h,v 1.1.1.1 2012/04/24 02:28:29 thki81 Exp $
 *
 * Copyright (c) 1995 Regents of The University of Michigan.
 * All Rights Reserved.  See COPYRIGHT.
 */

#ifndef PAPD_PPD_H
#define PAPD_PPD_H 1

#include <sys/cdefs.h>

struct ppd_font {
    char		*pd_font;
    struct ppd_font	*pd_next;
};

struct ppd_feature {
    char	*pd_name;
    char	*pd_value;
};

struct ppd_feature	*ppd_feature (const char *, int);
struct ppd_font		*ppd_font (char *);
int read_ppd (char *, int);

#endif /* PAPD_PPD_H */
