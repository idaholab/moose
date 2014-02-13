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

template<>
InputParameters validParams<DiffusionFluxBC>();

/**
 * \f$ F(u) = - \int_Gamma \grad u * \hat n * phi dGamma \f$
 */
class DiffusionFluxBC : public FluxBC
{
public:
  DiffusionFluxBC(const std::string & name, InputParameters parameters);
  virtual ~DiffusionFluxBC();

protected:
  virtual RealGradient computeQpFluxResidual();
  virtual RealGradient computeQpFluxJacobian();
};

#endif /* DIFFUSIONFLUXBC_H */
