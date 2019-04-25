//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VECTORFUNCTIONIC_H
#define VECTORFUNCTIONIC_H

#include "VectorInitialCondition.h"

// System includes
#include <string>

// Forward Declarations
class VectorFunctionIC;
class InputParameters;

namespace libMesh
{
class Point;
}

template <>
InputParameters validParams<VectorFunctionIC>();

/**
 * Vectorfunctionic just returns a constant value.
 */
class VectorFunctionIC : public VectorInitialCondition
{
public:
  VectorFunctionIC(const InputParameters & parameters);

  virtual RealVectorValue value(const Point & p) override;

protected:
  Function & _function;
};

#endif
