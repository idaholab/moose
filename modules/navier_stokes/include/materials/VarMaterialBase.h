#pragma once

#include "Material.h"

class SinglePhaseFluidProperties;

/**
 * Material to calculate nonlinear and auxiliary variables as material
 * properties to allow seamless use of different sets of solution
 * fields in other objects. This abstract base class simply defines
 * the materials that all derived classes must compute based on some
 * set of nonlinear variables.
 */
class VarMaterialBase : public Material
{
public:
  VarMaterialBase(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual void computeQpProperties() = 0;

  /// assign coupled nonlinear variables to material properties; this
  /// method simply sets one-to-one correspondence between nonlinear variables
  /// and material properties. More complicated calculations are placed in
  /// 'computeQpProperties'.
  virtual void setNonlinearProperties() = 0;

  /// whether the nonlinear variables coupled to this material are in
  /// the auxiliary variable system, in which case Jacobians will be incorrect
  virtual bool coupledAuxiliaryVariables() const = 0;

  /// print warning that jacobians are incorrect if nonlinear variables are
  /// in the auxiliary variable system
  void warnAuxiliaryVariables() const;

  /// compute the speed
  virtual ADReal computeSpeed() const;

  /// fluid properties
  const SinglePhaseFluidProperties & _fluid;

  /// density
  ADMaterialProperty<Real> & _rho;

  /// momentum
  ADMaterialProperty<RealVectorValue> & _momentum;

  ADMaterialProperty<Real> & _specific_internal_energy;
  ADMaterialProperty<Real> & _specific_total_energy;
  ADMaterialProperty<Real> & _total_energy_density;

  ADMaterialProperty<Real> & _specific_total_enthalpy;
  ADMaterialProperty<Real> & _total_enthalpy_density;

  /// specific volume
  ADMaterialProperty<Real> & _v;

  /// fluid temperature
  ADMaterialProperty<Real> & _T_fluid;

  /// pressure
  ADMaterialProperty<Real> & _pressure;

  /// speed (magnitude of velocity
  ADMaterialProperty<Real> & _speed;

  /// velocity
  ADMaterialProperty<RealVectorValue> & _velocity;

  /// density gradient
  ADMaterialProperty<RealVectorValue> & _grad_rho;

  /// total fluid energy gradient
  ADMaterialProperty<RealVectorValue> & _grad_rho_et;

  /// $x$-component of momentum gradient
  ADMaterialProperty<RealVectorValue> & _grad_rho_u;

  /// $y$-component of momentum gradient
  ADMaterialProperty<RealVectorValue> & _grad_rho_v;

  /// $z$-component of momentum gradient
  ADMaterialProperty<RealVectorValue> & _grad_rho_w;

  /// $x$-component of velocity gradient
  ADMaterialProperty<RealVectorValue> & _grad_vel_x;

  /// $y$-component of velocity gradient
  ADMaterialProperty<RealVectorValue> & _grad_vel_y;

  /// $z$-component of velocity gradient
  ADMaterialProperty<RealVectorValue> & _grad_vel_z;

  /// fluid temperature gradient
  ADMaterialProperty<RealVectorValue> & _grad_T_fluid;

  /// pressure gradient
  ADMaterialProperty<RealVectorValue> & _grad_pressure;

  /// density time derivative
  ADMaterialProperty<Real> & _drho_dt;

  /// total fluid energy time derivative
  ADMaterialProperty<Real> & _drho_et_dt;

  /// $x$-component of momentum time derivative
  ADMaterialProperty<Real> & _drho_u_dt;

  /// $y$-component of momentum time derivative
  ADMaterialProperty<Real> & _drho_v_dt;

  /// $z$-component of momentum time derivative
  ADMaterialProperty<Real> & _drho_w_dt;

  /// fluid temperature time derivative
  ADMaterialProperty<Real> & _dT_dt;

  /// fluid temperature second gradient
  ADMaterialProperty<RealTensorValue> & _grad_grad_T_fluid;

  /// $x$-component of velocity second gradient
  ADMaterialProperty<RealTensorValue> & _grad_grad_vel_x;

  /// $y$-component of velocity second gradient
  ADMaterialProperty<RealTensorValue> & _grad_grad_vel_y;

  /// $z$-component of velocity second gradient
  ADMaterialProperty<RealTensorValue> & _grad_grad_vel_z;

  using UserObjectInterface::getUserObject;
};
