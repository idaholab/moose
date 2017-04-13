/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWDESORPEDMASSVOLUMETRICEXPANSION_H
#define POROUSFLOWDESORPEDMASSVOLUMETRICEXPANSION_H

#include "TimeDerivative.h"
#include "PorousFlowDictator.h"

// Forward Declarations
class PorousFlowDesorpedMassVolumetricExpansion;

template <>
InputParameters validParams<PorousFlowDesorpedMassVolumetricExpansion>();

/**
 * Kernel = desorped_mass * d(volumetric_strain)/dt
 * which is not lumped to the nodes
 */
class PorousFlowDesorpedMassVolumetricExpansion : public TimeKernel
{
public:
  PorousFlowDesorpedMassVolumetricExpansion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// holds info on the Porous Flow variables
  const PorousFlowDictator & _dictator;

  /// The MOOSE variable number of the concentration variable
  const unsigned int _conc_var_number;

  /// The concentration variable
  const VariableValue & _conc;

  /// porosity
  const MaterialProperty<Real> & _porosity;

  /// d(porosity)/d(porous-flow variable)
  const MaterialProperty<std::vector<Real>> & _dporosity_dvar;

  /// d(porosity)/d(grad porous-flow variable)
  const MaterialProperty<std::vector<RealGradient>> & _dporosity_dgradvar;

  /// strain rate
  const MaterialProperty<Real> & _strain_rate_qp;

  /// d(strain rate)/d(porous-flow variable)
  const MaterialProperty<std::vector<RealGradient>> & _dstrain_rate_qp_dvar;

  /**
   * Derivative of the residual with respect to the Moose variable
   * with variable number jvar.
   * @param jvar take the derivative of the mass part of the residual wrt this variable number
   */
  Real computeQpJac(unsigned int jvar) const;
};

#endif // POROUSFLOWDESORPEDMASSVOLUMETRICEXPANSION_H
