/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSBASEFULLYSATURATEDDARCYBASE_H
#define POROUSBASEFULLYSATURATEDDARCYBASE_H

#include "Kernel.h"
#include "PorousFlowDictator.h"

class PorousFlowFullySaturatedDarcyBase;

template <>
InputParameters validParams<PorousFlowFullySaturatedDarcyBase>();

/**
 * Darcy advective flux for a fully-saturated,
 * single phase, single component fluid.
 * No upwinding or relative-permeability is used.
 */
class PorousFlowFullySaturatedDarcyBase : public Kernel
{
public:
  PorousFlowFullySaturatedDarcyBase(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /**
   * The mobility of the fluid = density / viscosity
   */
  virtual Real mobility() const;

  /**
   * The derivative of the mobility with respect to the porous-flow variable pvar
   * @param pvar Take the derivative with respect to this porous-flow variable
   */
  virtual Real dmobility(unsigned pvar) const;

  /// If true then the mobility contains the fluid density, otherwise it doesn't
  const bool _multiply_by_density;

  /// Permeability of porous material
  const MaterialProperty<RealTensorValue> & _permeability;

  /// d(permeabiity)/d(porous-flow variable)
  const MaterialProperty<std::vector<RealTensorValue>> & _dpermeability_dvar;

  /// d(permeabiity)/d(grad(porous-flow variable))
  const MaterialProperty<std::vector<std::vector<RealTensorValue>>> & _dpermeability_dgradvar;

  /// Fluid density for each phase (at the qp)
  const MaterialProperty<std::vector<Real>> & _density;

  /// Derivative of the fluid density for each phase wrt PorousFlow variables (at the qp)
  const MaterialProperty<std::vector<std::vector<Real>>> & _ddensity_dvar;

  /// Viscosity of the fluid at the qp
  const MaterialProperty<std::vector<Real>> & _viscosity;

  /// Derivative of the fluid viscosity  wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _dviscosity_dvar;

  /// Quadpoint pore pressure in each phase
  const MaterialProperty<std::vector<Real>> & _pp;

  /// Gradient of the pore pressure in each phase
  const MaterialProperty<std::vector<RealGradient>> & _grad_p;

  /// Derivative of Grad porepressure in each phase wrt grad(PorousFlow variables)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dgrad_p_dgrad_var;

  /// Derivative of Grad porepressure in each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<RealGradient>>> & _dgrad_p_dvar;

  /// PorousFlow UserObject
  const PorousFlowDictator & _porousflow_dictator;

  /// Gravity pointing downwards
  const RealVectorValue _gravity;
};

#endif // POROUSBASEFULLYSATURATEDDARCYBASE_H
