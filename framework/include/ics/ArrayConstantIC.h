//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ArrayInitialCondition.h"

// Forward Declarations
class ArrayConstantIC;
namespace libMesh
{
class Point;
}

template <>
InputParameters validParams<ArrayConstantIC>();

class ArrayConstantIC : public ArrayInitialCondition
{
public:
  ArrayConstantIC(const InputParameters & parameters);

  virtual RealEigenVector value(const Point & p) override;

protected:
  const RealEigenVector _value;
};
