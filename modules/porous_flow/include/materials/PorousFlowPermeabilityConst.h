//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWPERMEABILITYCONST_H
#define POROUSFLOWPERMEABILITYCONST_H

#include "PorousFlowPermeabilityBase.h"

// Forward Declarations
class PorousFlowPermeabilityConst;

template <>
InputParameters validParams<PorousFlowPermeabilityConst>();

/**
 * Material designed to provide a constant permeability tensor
 */
class PorousFlowPermeabilityConst : public PorousFlowPermeabilityBase
{
public:
  PorousFlowPermeabilityConst(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  /// constant value of permeability tensor
  const RealTensorValue _input_permeability;
};

#endif // POROUSFLOWPERMEABILITYCONST_H
