//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// This header is only here to avoid breaking CI during a merge, as apps are still relying on
// the old name for PropertyReadFile

#include "PropertyReadFile.h"
typedef PropertyReadFile ElementPropertyReadFile;
