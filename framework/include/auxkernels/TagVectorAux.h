//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "TagAuxBase.h"

/**
 * The value of a tagged vector for a given node and a given variable is coupled to
 * the current AuxVariable. TagVectorAux returns the coupled nodal value.
 */
class TagVectorAux : public TagAuxBase<AuxKernel>
{
public:
  static InputParameters validParams();

  TagVectorAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const VariableValue & _v;
  const MooseVariableBase & _v_var;
};
