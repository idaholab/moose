//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "UserObject.h"

/**
 * Interface class for user objects that interface with the generalized plane strain kernel.
 */
class GeneralizedPlaneStrainUOInterface
{
public:
  virtual Real returnResidual(unsigned int scalar_var_id = 0) const = 0;
  virtual Real returnReferenceResidual(unsigned int scalar_var_id = 0) const = 0;
  virtual Real returnJacobian(unsigned int scalar_var_id = 0) const = 0;
};
