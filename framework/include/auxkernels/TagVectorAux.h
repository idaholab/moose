//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * TagVectorAux returns the coupled DOF value of a tagged vector.
 */
class TagVectorAux : public TagAuxBase<AuxKernel>
{
public:
  static InputParameters validParams();

  TagVectorAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Whether to remove variable scaling from the returned value
  const bool _unscaled;
  const VariableValue & _v;
  const MooseVariableBase & _v_var;
};
