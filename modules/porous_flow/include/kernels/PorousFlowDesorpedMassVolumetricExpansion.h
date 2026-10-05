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
 * Kernel = desorped_mass * rate_of_solid_volumetric_expansion
 * where desorped_mass = (1 - porosity) * concentration
 */
template <bool is_ad>
class PorousFlowDesorpedMassVolumetricExpansionTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowDesorpedMassVolumetricExpansionTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /**
   * Derivative of the residual with respect to the Moose variable
   * with variable number jvar.
   * @param jvar take the derivative of the mass part of the residual wrt this variable number
   */
  Real computeQpJac(unsigned int jvar) const;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// MOOSE variable number of the concentration variable
  const unsigned int _conc_var_number;

  /// Concentration of the desorped species
  const GenericVariableValue<is_ad> & _conc;

  /// Porosity at the qps
  const GenericMaterialProperty<Real, is_ad> & _porosity;

  /// d(porosity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<Real>> * const _dporosity_dvar;

  /// d(porosity)/d(grad PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> * const _dporosity_dgradvar;

  /// Strain rate
  const GenericMaterialProperty<Real, is_ad> & _strain_rate_qp;

  /// d(strain rate)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> * const _dstrain_rate_qp_dvar;

  usingGenericKernelMembers;
  using GenericKernel<is_ad>::_grad_phi;
};

typedef PorousFlowDesorpedMassVolumetricExpansionTempl<false>
    PorousFlowDesorpedMassVolumetricExpansion;
typedef PorousFlowDesorpedMassVolumetricExpansionTempl<true>
    ADPorousFlowDesorpedMassVolumetricExpansion;
