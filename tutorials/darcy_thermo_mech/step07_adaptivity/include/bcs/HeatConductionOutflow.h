//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Include the base class so it can be extended
#include "ADIntegratedBC.h"

// Forward declare the class being created and the validParams function
template <ComputeStage>
class HeatConductionOutflow;

declareADValidParams(HeatConductionOutflow);

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
template <ComputeStage compute_stage>
class HeatConductionOutflow : public ADIntegratedBC<compute_stage>
{
public:
  HeatConductionOutflow(const InputParameters & parameters);

protected:
  /**
   * This is called to integrate the residual across the boundary.
   */
  virtual ADResidual computeQpResidual() override;

  /// Thermal conductivity of the material
  const MaterialProperty<Real> & _thermal_conductivity;

  usingIntegratedBCMembers;
};
