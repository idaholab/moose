//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "Moose.h"

/// Exec flag used to execute MooseObjects while elements are being
/// marked for cutting by XFEM
extern const ExecFlagType EXEC_XFEM_MARK;

// XFEM typedefs
typedef unsigned int CutSubdomainID;
