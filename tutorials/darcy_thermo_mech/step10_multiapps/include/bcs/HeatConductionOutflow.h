/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef HEATCONDUCTIONOUTFLOW_H
#define HEATCONDUCTIONOUTFLOW_H

#include "IntegratedBC.h"


class HeatConductionOutflow;

template<>
InputParameters validParams<HeatConductionOutflow>();

/**
 * An IntegratedBC representing the "No BC" boundary condition for Heat Conduction.
 *
 * This is essentially -test*k*grad_u*normal... essentially completing the integration by parts.
 * This is a well accepted practice for truncating longer domains for convection/diffusion
 * problems as analyzed in: Griffiths, David F. "The ‘no boundary condition’outflow boundary condition." International journal for numerical methods in fluids 24.4 (1997): 393-411.
 */
class HeatConductionOutflow : public IntegratedBC
{
public:
  HeatConductionOutflow(const InputParameters & parameters);


protected:
  /// This is called to integrate the residual across the boundary
  virtual Real computeQpResidual() override;

  /// Optional (but recommended!) to compute the derivative of the residual with respect to _this_ variable
  virtual Real computeQpJacobian() override;

  /// Thermal conductivity of the material
  const MaterialProperty<Real> & _thermal_conductivity;
};


#endif //HEATCONDUCTIONOUTFLOW_H
