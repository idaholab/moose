//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/// macro for source compatibility with UMAT plugins developed for a commercial code
#define FOR_NAME(lower, UPPER) lower##_
#define CALL_NAME(lower, UPPER) lower##_

typedef char * Character;
typedef int Length;
#define PTR(id) id
#define LEN(id, len) len
#define GETLEN(id) id##_len
#define CHNAME(id) Character id
#define CHNAME_C(id) id
#define CHLEN(id) , const Length id##_len
#define CHLEN_C(id) , id##_len
