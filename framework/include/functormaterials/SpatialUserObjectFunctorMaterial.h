//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "FunctorMaterial.h"

class UserObject;

/**
 * A functor material to create a functor material property from a spatial user object
 */
class SpatialUserObjectFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  SpatialUserObjectFunctorMaterial(const InputParameters & parameters);

protected:
  /// User object providing the spatial value
  const UserObject & _user_object;
};
