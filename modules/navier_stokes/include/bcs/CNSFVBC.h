/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVBC_H
#define CNSFVBC_H

#include "IntegratedBC.h"
#include "BoundaryFluxBase.h"

// Forward Declarations
class CNSFVBC;

template <>
InputParameters validParams<CNSFVBC>();

/**
 * A boundary condition object for the CNS equations
 *
 * Notes:
 *
 *   1. This BC does not do any complex calculation.
 *      It gets the flux vector and Jacobian matrix
 *      from the boundary flux user object being called.
 *
 *   2. If a system of governing equations is being solved,
 *      the flux vector and Jacobian matrix
 *      are calculated only once for the first equation
 *      and cached for use for the rest of the equations in the system.
 *
 *   3. On the "left" state of the boundary face, the variable value
 *      is interpolated from the reconstructed linear polynomial in the host element,
 *      which is provided from the corresponding material kernel.
 *
 *   4. On the "right" state of the boundary face, the variable value
 *      should be obtained from the bc user object being called.
 *
 */
class CNSFVBC : public IntegratedBC
{
public:
  CNSFVBC(const InputParameters & parameters);
  virtual ~CNSFVBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// choose an equation
  MooseEnum _component;

  /// "1" denotes the "host" state
  /// "2" denotes the "ghost" state

  /// piecewise constant variable values in host cells
  const VariableValue & _rhoc1;
  const VariableValue & _rhouc1;
  const VariableValue & _rhovc1;
  const VariableValue & _rhowc1;
  const VariableValue & _rhoec1;

  /// extrapolated variable values at side center
  const MaterialProperty<Real> & _rho1;
  const MaterialProperty<Real> & _rhou1;
  const MaterialProperty<Real> & _rhov1;
  const MaterialProperty<Real> & _rhow1;
  const MaterialProperty<Real> & _rhoe1;

  /// fluid properties object
  const BoundaryFluxBase & _flux;

  unsigned int _rho_var;
  unsigned int _rhou_var;
  unsigned int _rhov_var;
  unsigned int _rhow_var;
  unsigned int _rhoe_var;

  std::map<unsigned int, unsigned int> _jmap;
};

#endif
