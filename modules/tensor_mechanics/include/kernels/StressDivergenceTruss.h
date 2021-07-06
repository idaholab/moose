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
#include "RankTwoTensorForward.h"

class StressDivergenceTruss : public Kernel
{
public:
  static InputParameters validParams();

  StressDivergenceTruss(const InputParameters & parameters);
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  virtual void initialSetup() override;
  virtual Real computeStiffness(unsigned int i, unsigned int j);
  virtual Real computeQpResidual() override { return 0.0; }

  /// Direction along which force is calculated
  const unsigned int _component;

  /// Number of coupled displacement variables
  unsigned int _ndisp;

  /// Variable numbers corresponding to displacement variables
  std::vector<unsigned int> _disp_var;

  /// Current force vector in global coordinate system
  const MaterialProperty<Real> & _force;

  /// Stiffness matrix relating displacement DOFs of same node or across nodes
  const MaterialProperty<Real> & _e_over_l;

  const std::vector<RealGradient> * _orientation;
};
