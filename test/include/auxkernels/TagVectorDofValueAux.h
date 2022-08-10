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

class TagVectorDofValueAux : public TagAuxBase<AuxKernel>
{
public:
  static InputParameters validParams();

  TagVectorDofValueAux(const InputParameters & parameters);

protected:
  void compute() override;
  Real computeValue() override final { mooseError("Work is implemented in compute"); }

  /// The result of evaluating the supplied tagged vector at the degrees of freedom corresponding to
  /// the provided variable
  const VariableValue & _v;

  using TagAuxBase<AuxKernel>::coupledVectorTagDofValue;
  using TagAuxBase<AuxKernel>::getFieldVar;
};
