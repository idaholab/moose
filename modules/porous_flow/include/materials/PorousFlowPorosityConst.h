//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWPOROSITYCONST_H
#define POROUSFLOWPOROSITYCONST_H

#include "PorousFlowPorosityBase.h"

// Forward Declarations
class PorousFlowPorosityConst;

template <>
InputParameters validParams<PorousFlowPorosityConst>();

/**
 * Material to provide a constant value of porosity. This can be specified
 * by either a constant value in the input file, or taken from an aux variable.
 * Note: this material assumes that the porosity remains constant throughout a
 * simulation, so the coupled aux variable porosity must also remain constant.
 */
class PorousFlowPorosityConst : public PorousFlowPorosityBase
{
public:
  PorousFlowPorosityConst(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Constant porosity
  const VariableValue & _input_porosity;
};

#endif // POROUSFLOWPOROSITYCONST_H
