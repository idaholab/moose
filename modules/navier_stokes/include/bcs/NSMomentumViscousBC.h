//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSIntegratedBC.h"
#include "NSViscStressTensorDerivs.h"

// Forward Declarations

/**
 * This class corresponds to the viscous part of the "natural"
 * boundary condition for the momentum equations, i.e.
 *
 * int_{Gamma} n . (-tau) . v
 *
 * A typical use for this kernel would be a subsonic outflow BC in
 * which one physical value must be specified.  In this case, the
 * residual and Jacobian contrbutions of the n.tau.v term is computed
 * and added to the matrix/rhs.
 */
class NSMomentumViscousBC : public NSIntegratedBC
{
public:
  static InputParameters validParams();

  NSMomentumViscousBC(const InputParameters & parameters);

protected:
  /**
   * Just like other kernels, we must overload the Residual and Jacobian contributions...
   */
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Which spatial component of the momentum equations (0,1, or 2) is this
  // kernel applied in?
  const unsigned _component;

  // An object for computing viscous stress tensor derivatives.
  // Constructed via a reference to ourself so we can access all of our data.
  NSViscStressTensorDerivs<NSMomentumViscousBC> _vst_derivs;

  // Declare ourselves friend to the helper class.
  template <class U>
  friend class NSViscStressTensorDerivs;
};
