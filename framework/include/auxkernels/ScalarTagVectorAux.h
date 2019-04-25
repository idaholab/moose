//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxScalarKernel.h"

// Forward Declarations
class ScalarTagVectorAux;

template <>
InputParameters validParams<AuxScalarKernel>();

/**
 * The value of a tagged vector for a given node and a given variable is coupled to
 * the current AuxVariable. ScalarTagVectorAux returns the coupled value.
 */
class ScalarTagVectorAux : public AuxScalarKernel
{
public:
  ScalarTagVectorAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  TagID _tag_id;
  const VariableValue & _v;
};

