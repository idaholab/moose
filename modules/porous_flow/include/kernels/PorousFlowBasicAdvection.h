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
#include "PorousFlowDictator.h"

/**
 * Kernel = grad(test) * darcy_velocity * u
 */
class PorousFlowBasicAdvection : public Kernel
{
public:
  static InputParameters validParams();

  PorousFlowBasicAdvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Holds info on the Porous Flow variables
  const PorousFlowDictator & _dictator;

  /// Phase of Darcy velocity
  const unsigned _ph;

  /// _darcy_velocity[_qp][ph](j) = j^th component of the Darcy velocity of phase ph
  const MaterialProperty<std::vector<RealVectorValue>> & _darcy_velocity;

  /**
   * _ddarcy_velocity_dvar[_qp][ph][v](j)
   *  = d(j^th component of the Darcy velocity of phase ph)/d(PorousFlow variable v)
   */
  const MaterialProperty<std::vector<std::vector<RealVectorValue>>> & _ddarcy_velocity_dvar;

  /**
   * _ddarcy_velocity_dgradvar[_qp][ph][j][v](k)
   *  = d(k^th component of the Darcy velocity of phase ph)/d(j^th component of grad(PorousFlow
   * variable v))
   */
  const MaterialProperty<std::vector<std::vector<std::vector<RealVectorValue>>>> &
      _ddarcy_velocity_dgradvar;
};
