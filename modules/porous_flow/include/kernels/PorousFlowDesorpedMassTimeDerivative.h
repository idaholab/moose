/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWDESORPEDMASSTIMEDERIVATIVE_H
#define POROUSFLOWDESORPEDMASSTIMEDERIVATIVE_H

#include "TimeDerivative.h"
#include "PorousFlowDictator.h"

// Forward Declarations
class PorousFlowDesorpedMassTimeDerivative;

template <>
InputParameters validParams<PorousFlowDesorpedMassTimeDerivative>();

/**
 * Kernel = (desorped_mass - desorped_mass_old)/dt
 * It is NOT lumped to the nodes
 */
class PorousFlowDesorpedMassTimeDerivative : public TimeKernel
{
public:
  PorousFlowDesorpedMassTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// holds info on the PorousFlow variables
  const PorousFlowDictator & _dictator;

  /// The MOOSE variable number of the concentration variable
  const unsigned int _conc_var_number;

  /// The concentration variable
  const VariableValue & _conc;

  /// Old value of the concentration variable
  const VariableValue & _conc_old;

  /// porosity at the qps
  const MaterialProperty<Real> & _porosity;

  /// old value of porosity
  const MaterialProperty<Real> & _porosity_old;

  /// d(porosity)/d(porous-flow variable) - these derivatives will be wrt variables at the qps
  const MaterialProperty<std::vector<Real>> & _dporosity_dvar;

  /// d(porosity)/d(grad porous-flow variable) - these derivatives will be wrt grad(vars) at qps
  const MaterialProperty<std::vector<RealGradient>> & _dporosity_dgradvar;

  /**
   * Derivative of residual with respect to variable number jvar
   * This is used by both computeQpJacobian and computeQpOffDiagJacobian
   * @param jvar take the derivative of the residual wrt this Moose variable
   */
  Real computeQpJac(unsigned int jvar) const;
};

#endif // POROUSFLOWDESORPEDMASSTIMEDERIVATIVE_H
