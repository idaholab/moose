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
 * Couple a tagged vector, and return its evaluations at degree of freedom indices corresponding to
 * the coupled array variable
 */
class TagVectorArrayVariableAux : public TagAuxBase<ArrayAuxKernel>
{
public:
  static InputParameters validParams();

  TagVectorArrayVariableAux(const InputParameters & parameters);

protected:
  void compute() override;
  RealEigenVector computeValue() override final { mooseError("Work is implemented in compute"); }

  /// The result of evaluating the supplied tagged vector at the degrees of freedom corresponding to
  /// the provided array variable
  const ArrayVariableValue & _v;

  using TagAuxBase<ArrayAuxKernel>::coupledVectorTagArrayDofValue;
  using TagAuxBase<ArrayAuxKernel>::getArrayVar;
};
