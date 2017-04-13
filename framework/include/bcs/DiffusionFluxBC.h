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

#ifndef DIFFUSIONFLUXBC_H
#define DIFFUSIONFLUXBC_H

#include "FluxBC.h"

class DiffusionFluxBC;

template <>
InputParameters validParams<DiffusionFluxBC>();

/**
 * \f$ F(u) = - \int_{\Gamma} \nabla u * \hat n * \phi d\Gamma \f$
 */
class DiffusionFluxBC : public FluxBC
{
public:
  DiffusionFluxBC(const InputParameters & parameters);

protected:
  virtual RealGradient computeQpFluxResidual() override;
  virtual RealGradient computeQpFluxJacobian() override;
};

#endif /* DIFFUSIONFLUXBC_H */
