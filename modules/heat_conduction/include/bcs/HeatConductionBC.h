/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEATCONDUCTIONBC_H
#define HEATCONDUCTIONBC_H

#include "FluxBC.h"

class HeatConductionBC;

template <>
InputParameters validParams<HeatConductionBC>();

/**
 *
 */
class HeatConductionBC : public FluxBC
{
public:
  HeatConductionBC(const InputParameters & parameters);
  virtual ~HeatConductionBC();

protected:
  virtual RealGradient computeQpFluxResidual();
  virtual RealGradient computeQpFluxJacobian();

  const MaterialProperty<Real> & _k;
};

#endif /* HEATCONDUCTIONBC_H */
