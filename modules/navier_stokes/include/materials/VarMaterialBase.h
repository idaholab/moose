#pragma once

#include "ADMaterial.h"

#define usingVarMaterialBaseMembers                                                      \
  using TransientInterface::_is_transient;                                               \
  using VarMaterialBase::_fluid;                                          \
  using VarMaterialBase::_rho;                                            \
  using VarMaterialBase::_rhoE;                                           \
  using VarMaterialBase::_momentum;                                       \
  using VarMaterialBase::_enthalpy;                                       \
  using VarMaterialBase::_e;                                              \
  using VarMaterialBase::_v;                                              \
  using VarMaterialBase::_T_fluid;                                        \
  using VarMaterialBase::_pressure;                                       \
  using VarMaterialBase::_speed;                                          \
  using VarMaterialBase::_velocity;                                       \
  using VarMaterialBase::_grad_rho;                                       \
  using VarMaterialBase::_grad_rhoE;                                      \
  using VarMaterialBase::_grad_rho_u;                                     \
  using VarMaterialBase::_grad_rho_v;                                     \
  using VarMaterialBase::_grad_rho_w;                                     \
  using VarMaterialBase::_grad_vel_x;                                     \
  using VarMaterialBase::_grad_vel_y;                                     \
  using VarMaterialBase::_grad_vel_z;                                     \
  using VarMaterialBase::_grad_T_fluid;                                   \
  using VarMaterialBase::_grad_pressure;                                  \
  using VarMaterialBase::_drho_dt;                                        \
  using VarMaterialBase::_drhoE_dt;                                       \
  using VarMaterialBase::_drho_u_dt;                                      \
  using VarMaterialBase::_drho_v_dt;                                      \
  using VarMaterialBase::_drho_w_dt;                                      \
  using VarMaterialBase::_dT_dt;                                          \
  using VarMaterialBase::_grad_grad_T_fluid;                              \
  using VarMaterialBase::_grad_grad_vel_x;                                \
  using VarMaterialBase::_grad_grad_vel_y;                                \
  using VarMaterialBase::_grad_grad_vel_z;                                \
  using VarMaterialBase::computeSpeed

class VarMaterialBase;
class SinglePhaseFluidProperties;

declareADValidParams(VarMaterialBase);

/**
 * Material to calculate nonlinear and auxiliary variables as material
 * properties to allow seamless use of different sets of solution
 * fields in other objects. This abstract base class simply defines
 * the materials that all derived classes must compute based on some
 * set of nonlinear variables.
 */
class VarMaterialBase : public ADMaterial
{
public:
  VarMaterialBase(const InputParameters & parameters);

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

  /// total fluid energy
  ADMaterialProperty<Real> & _rhoE;

  /// momentum
  ADMaterialProperty<RealVectorValue> & _momentum;

  /// total enthalpy
  ADMaterialProperty<Real> & _enthalpy;

  /// internal energy
  ADMaterialProperty<Real> & _e;

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
  ADMaterialProperty<RealVectorValue> & _grad_rhoE;

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
  ADMaterialProperty<Real> & _drhoE_dt;

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
