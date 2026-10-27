//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"
#include "PorousFlowDictator.h"

/**
 * Kernel = grad(test) * darcy_velocity * u
 *
 * Templated on is_ad: the false instantiation uses the hand-coded Jacobian;
 * the true instantiation propagates derivatives through the AD Darcy velocity and u.
 */
template <bool is_ad>
class PorousFlowBasicAdvectionTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowBasicAdvectionTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Holds info on the Porous Flow variables
  const PorousFlowDictator & _dictator;

  /// Phase of Darcy velocity
  const unsigned _ph;

  /// _darcy_velocity[_qp][ph](j) = j^th component of the Darcy velocity of phase ph
  const GenericMaterialProperty<std::vector<RealVectorValue>, is_ad> & _darcy_velocity;

  /**
   * _ddarcy_velocity_dvar[_qp][ph][v](j)
   *  = d(j^th component of the Darcy velocity of phase ph)/d(PorousFlow variable v)
   * Null for the AD path.
   */
  const MaterialProperty<std::vector<std::vector<RealVectorValue>>> * const _ddarcy_velocity_dvar;

  /**
   * _ddarcy_velocity_dgradvar[_qp][ph][j][v](k)
   *  = d(k^th component of the Darcy velocity of phase ph)/d(j^th component of grad(PorousFlow
   * variable v))
   * Null for the AD path.
   */
  const MaterialProperty<std::vector<std::vector<std::vector<RealVectorValue>>>> * const
      _ddarcy_velocity_dgradvar;

  usingGenericKernelMembers;
  using GenericKernel<is_ad>::_grad_phi;
};

typedef PorousFlowBasicAdvectionTempl<false> PorousFlowBasicAdvection;
typedef PorousFlowBasicAdvectionTempl<true> ADPorousFlowBasicAdvection;
