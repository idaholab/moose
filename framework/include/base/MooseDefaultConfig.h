//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/* framework/include/base/MooseConfig.h. Generated automatically at end of configure. */
/* framework/include/base/MooseConfig.h.tmp.  Generated from MooseConfig.h.in by configure.  */
/* framework/include/base/MooseConfig.h.in.  Generated from configure.ac by autoheader.  */

/* The size of the derivative backing array */
#ifndef MOOSE_AD_MAX_DOFS_PER_ELEM
#define MOOSE_AD_MAX_DOFS_PER_ELEM 53
#endif

/* Whether to use a global indexing scheme for AD */
#ifndef MOOSE_GLOBAL_AD_INDEXING
#define MOOSE_GLOBAL_AD_INDEXING 1
#endif

/* Whether or not libpng was detected on the system */
/* #undef HAVE_LIBPNG */

/* Define to the address where bug reports for this package should be sent. */
#ifndef MOOSE_PACKAGE_BUGREPORT
#define MOOSE_PACKAGE_BUGREPORT "moose-users@googlegroups.com"
#endif

/* Define to the full name of this package. */
#ifndef MOOSE_PACKAGE_NAME
#define MOOSE_PACKAGE_NAME "moose"
#endif

/* Define to the full name and version of this package. */
#ifndef MOOSE_PACKAGE_STRING
#define MOOSE_PACKAGE_STRING "moose 0.9.0"
#endif

/* Define to the one symbol short name of this package. */
#ifndef MOOSE_PACKAGE_TARNAME
#define MOOSE_PACKAGE_TARNAME "moose"
#endif

/* Define to the home page for this package. */
#ifndef MOOSE_PACKAGE_URL
#define MOOSE_PACKAGE_URL "https://mooseframework.org"
#endif

/* Define to the version of this package. */
#ifndef MOOSE_PACKAGE_VERSION
#define MOOSE_PACKAGE_VERSION "0.9.0"
#endif

/* Whether to use a sparse derivative type */
#ifndef MOOSE_SPARSE_AD
#define MOOSE_SPARSE_AD 1
#endif
