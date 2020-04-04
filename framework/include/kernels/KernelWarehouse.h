//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectWarehouse.h"

/// This is only here for backwards compatibility.  Can be removed anytime after 9/1/2018
typedef MooseObjectWarehouse<KernelBase> KernelWarehouse;
