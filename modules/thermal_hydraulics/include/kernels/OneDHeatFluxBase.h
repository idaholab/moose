//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

class HeatFluxFromHeatStructureBaseUserObject;

class OneDHeatFluxBase : public Kernel
{
public:
  OneDHeatFluxBase(const InputParameters & parameters);

  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned jvar) override;

protected:
  virtual Real computeQpOffDiagJacobianNeighbor(unsigned int jvar) = 0;

  /// shape function values (in QPs)
  const VariablePhiValue & _phi_neighbor;
  /// User object that computes the heat flux
  const HeatFluxFromHeatStructureBaseUserObject & _q_uo;

public:
  static InputParameters validParams();
};
