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

/**
 * This class computes strong and weak components of the INS governing
 * equations.  These terms can then be assembled in child classes
 */
class INSBase : public Kernel
{
public:
  static InputParameters validParams();

  INSBase(const InputParameters & parameters);

  virtual ~INSBase() {}

protected:
  virtual Real computeQpResidual() = 0;
  virtual Real computeQpJacobian() = 0;
  virtual Real computeQpOffDiagJacobian(unsigned jvar) = 0;

  virtual RealVectorValue convectiveTerm();
  virtual RealVectorValue dConvecDUComp(unsigned comp);

  virtual RealVectorValue strongViscousTermLaplace();
  virtual RealVectorValue strongViscousTermTraction();
  virtual RealVectorValue dStrongViscDUCompLaplace(unsigned comp);
  virtual RealVectorValue dStrongViscDUCompTraction(unsigned comp);

  virtual RealVectorValue weakViscousTermLaplace(unsigned comp);
  virtual RealVectorValue weakViscousTermTraction(unsigned comp);
  virtual RealVectorValue dWeakViscDUCompLaplace();
  virtual RealVectorValue dWeakViscDUCompTraction();

  virtual RealVectorValue strongPressureTerm();
  virtual Real weakPressureTerm();
  virtual RealVectorValue dStrongPressureDPressure();
  virtual Real dWeakPressureDPressure();

  virtual RealVectorValue gravityTerm();

  virtual RealVectorValue timeDerivativeTerm();
  virtual RealVectorValue dTimeDerivativeDUComp(unsigned comp);

  virtual Real tau();
  virtual Real dTauDUComp(unsigned comp);

  /// Provides tau which yields superconvergence for 1D advection-diffusion
  virtual Real tauNodal();

  /**
   * Compute the velocity. If displacements are provided, then this routine will subtract the time
   * derivative of the displacements, e.g. the mesh velocity
   */
  RealVectorValue relativeVelocity() const;

  /**
   * Computes the additional RZ terms for the Laplace form of the strong viscous term
   */
  RealVectorValue strongViscousTermLaplaceRZ() const;

  /**
   * Computes the Jacobian for the additional RZ terms for the Laplace form of the strong viscous
   * term for the given velocity component \p comp
   */
  RealVectorValue dStrongViscDUCompLaplaceRZ(const unsigned int comp) const;

  /**
   * Computes the additional RZ terms for the Traction form of the strong viscous term
   */
  RealVectorValue strongViscousTermTractionRZ() const;

  /**
   * Computes the Jacobian for the additional RZ terms for the Traction form of the strong viscous
   * term for the given velocity component \p comp
   */
  RealVectorValue dStrongViscDUCompTractionRZ(const unsigned int comp) const;

  /// second derivatives of the shape function
  const VariablePhiSecond & _second_phi;

  // Coupled variables
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
  const VariableValue & _p;

  const bool _picard;
  const VariableValue * const _u_vel_previous_nl;
  const VariableValue * const _v_vel_previous_nl;
  const VariableValue * const _w_vel_previous_nl;

  // Gradients
  const VariableGradient & _grad_u_vel;
  const VariableGradient & _grad_v_vel;
  const VariableGradient & _grad_w_vel;
  const VariableGradient & _grad_p;

  // Seconds
  const VariableSecond & _second_u_vel;
  const VariableSecond & _second_v_vel;
  const VariableSecond & _second_w_vel;

  // Time derivatives
  const VariableValue & _u_vel_dot;
  const VariableValue & _v_vel_dot;
  const VariableValue & _w_vel_dot;

  // Derivatives of time derivatives
  const VariableValue & _d_u_vel_dot_du;
  const VariableValue & _d_v_vel_dot_dv;
  const VariableValue & _d_w_vel_dot_dw;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;
  unsigned _p_var_number;

  RealVectorValue _gravity;

  // Material properties
  const MaterialProperty<Real> & _mu;
  const MaterialProperty<Real> & _rho;

  const Real & _alpha;
  bool _laplace;
  bool _convective_term;
  bool _transient_term;

  /// Whether displacements are provided
  const bool _disps_provided;
  /// Time derivative of the x-displacement, mesh velocity in the x-direction
  const VariableValue & _disp_x_dot;
  /// Time derivative of the y-displacement, mesh velocity in the y-direction
  const VariableValue & _disp_y_dot;
  /// Time derivative of the z-displacement, mesh velocity in the z-direction
  const VariableValue & _disp_z_dot;

  /// The radial coordinate index for RZ coordinate systems
  const unsigned int _rz_radial_coord;
};
