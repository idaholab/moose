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

#ifndef FLUXBC_H
#define FLUXBC_H

#include "IntegratedBC.h"

class FluxBC;

template <>
InputParameters validParams<FluxBC>();

/**
 * Boundary condition of a flux type \f$ <\vec q * \vec n, v> \f$.
 *
 * User needs to provide vector \f$ \vec q \f$.
 */
class FluxBC : public IntegratedBC
{
public:
  FluxBC(const InputParameters & params);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  virtual RealGradient computeQpFluxResidual() = 0;
  virtual RealGradient computeQpFluxJacobian() = 0;
};

#endif /* FLUXBC_H */
