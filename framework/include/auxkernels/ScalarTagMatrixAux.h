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
#include "TagAuxBase.h"

/**
 * The value of a tagged matrix for a given node and a given variable is coupled to
 * the current AuxVariable. ScalarTagMatrixAux returns the coupled nodal value.
 */
class ScalarTagMatrixAux : public TagAuxBase<AuxScalarKernel>
{
public:
  static InputParameters validParams();

  ScalarTagMatrixAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  TagID _tag_id;
  const VariableValue & _v;
  const MooseVariableScalar & _v_var;
};
