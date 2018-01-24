/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef AEFVKERNEL_H
#define AEFVKERNEL_H

#include "DGKernel.h"
#include "InternalSideFluxBase.h"

class AEFVKernel;

template <>
InputParameters validParams<AEFVKernel>();

/**
 * A dgkernel for the advection equation
 * using a cell-centered finite volume method
 *
 * Notes:
 *
 *   1. This dgkernel itself does not do any complex calculation.
 *      It gets the internal side flux vector and Jacobian matrix
 *      from the internal side flux user object being called.
 *
 *   2. In general, a approximate Riemann solver should be used
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
class AEFVKernel : public DGKernel
{
public:
  AEFVKernel(const InputParameters & parameters);
  virtual ~AEFVKernel();

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type);
  virtual Real computeQpJacobian(Moose::DGJacobianType type);

  /// choose an equation
  MooseEnum _component;

  // "1" denotes the "left" state
  // "2" denotes the "right" state

  /// piecewise constant variable values in cells
  const VariableValue & _uc1;
  const VariableValue & _uc2;

  /// extrapolated variable values at side center
  const MaterialProperty<Real> & _u1;
  const MaterialProperty<Real> & _u2;

  /// flux user object
  const InternalSideFluxBase & _flux;
};

#endif
