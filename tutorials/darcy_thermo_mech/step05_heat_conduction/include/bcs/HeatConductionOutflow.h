//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef HEATCONDUCTIONOUTFLOW_H
#define HEATCONDUCTIONOUTFLOW_H

#include "IntegratedBC.h"

class HeatConductionOutflow;

template <>
InputParameters validParams<HeatConductionOutflow>();

/**
 * An IntegratedBC representing the "No BC" boundary condition for
 * heat conduction.
 *
 * The residual is simply -test*k*grad_u*normal... the term you get
 * from integration by parts.  This is a standard technique for
 * truncating longer domains when solving the convection/diffusion
 * equation.
 *
 * See also: Griffiths, David F. "The 'no boundary condition' outflow
 * boundary condition.", International Journal for Numerical Methods
 * in Fluids, vol. 24, no. 4, 1997, pp. 393-411.
 */
class HeatConductionOutflow : public IntegratedBC
{
public:
  HeatConductionOutflow(const InputParameters & parameters);

protected:
  /**
   * This is called to integrate the residual across the boundary.
   */
  virtual Real computeQpResidual() override;

  /**
   * Optional (but recommended!) to compute the derivative of the
   * residual with respect to _this_ variable.
   */
  virtual Real computeQpJacobian() override;

  /// Thermal conductivity of the material
  const MaterialProperty<Real> & _thermal_conductivity;
};

#endif // HEATCONDUCTIONOUTFLOW_H
