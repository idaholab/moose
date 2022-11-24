//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TagAuxBase.h"
#include "AuxKernel.h"

/**
 * Couple a tagged vector, and return a component of its array gradient
 */
class TagVectorArrayVariableGradientAux : public TagAuxBase<ArrayAuxKernel>
{
public:
  static InputParameters validParams();

  TagVectorArrayVariableGradientAux(const InputParameters & parameters);

protected:
  RealEigenVector computeValue() override;

  const ArrayVariableValue & _v;

  const ArrayVariableGradient & _v_grad;

  const unsigned int _grad_component;
};
