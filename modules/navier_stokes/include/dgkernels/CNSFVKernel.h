/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVKERNEL_H
#define CNSFVKERNEL_H

#include "DGKernel.h"
#include "InternalSideFluxBase.h"

class CNSFVKernel;

template <>
InputParameters validParams<CNSFVKernel>();

/**
 * A DGKernel for the CNS equations
 *
 * Notes:
 *
 *   1. This dgkernel itself does not do any complex calculation.
 *      It gets the internal side flux vector and Jacobian matrix
 *      from the internal side flux user object being called.
 *
 *   2. In general, an approximate Riemann solver should be used
 *      in the internal flux user object for calculating the flux
 *
 *   3. If a system of governing equations is being solved,
 *      the flux vector and Jacobian matrix
 *      are calculated only once for the first equation
 *      and cached for use for the rest of the equations in the system.
 *
 *   4. On the "left" and "right" states of the internal side, the variable values
 *      are interpolated from the reconstructed linear polynomials
 *      in the "left" and "right "element, respectively,
 *      which are provided from the corresponding material kernel.
 */
class CNSFVKernel : public DGKernel
{
public:
  CNSFVKernel(const InputParameters & parameters);
  virtual ~CNSFVKernel();

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type);
  virtual Real computeQpJacobian(Moose::DGJacobianType type);
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar);

  /// choose an equation
  MooseEnum _component;

  // "1" denotes the "left" state
  // "2" denotes the "right" state

  // piecewise constant variable values in cells
  const VariableValue & _rhoc1;
  const VariableValue & _rhouc1;
  const VariableValue & _rhovc1;
  const VariableValue & _rhowc1;
  const VariableValue & _rhoec1;
  const VariableValue & _rhoc2;
  const VariableValue & _rhouc2;
  const VariableValue & _rhovc2;
  const VariableValue & _rhowc2;
  const VariableValue & _rhoec2;

  // extrapolated variable values at side center
  const MaterialProperty<Real> & _rho1;
  const MaterialProperty<Real> & _rhou1;
  const MaterialProperty<Real> & _rhov1;
  const MaterialProperty<Real> & _rhow1;
  const MaterialProperty<Real> & _rhoe1;
  const MaterialProperty<Real> & _rho2;
  const MaterialProperty<Real> & _rhou2;
  const MaterialProperty<Real> & _rhov2;
  const MaterialProperty<Real> & _rhow2;
  const MaterialProperty<Real> & _rhoe2;

  /// fluid properties object
  const InternalSideFluxBase & _flux;

  unsigned int _rho_var;
  unsigned int _rhou_var;
  unsigned int _rhov_var;
  unsigned int _rhow_var;
  unsigned int _rhoe_var;

  std::map<unsigned int, unsigned int> _jmap;
};

#endif
