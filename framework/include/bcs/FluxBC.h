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

template<>
InputParameters validParams<FluxBC>();

/**
 * Boundary condition of a flux type \f$ <\arrow q * \arrow n, v> \f$.
 *
 * User needs to provide vector \f$ \arrow q \f$.
 */
class FluxBC : public IntegratedBC
{
public:
  FluxBC(const std::string & name, InputParameters params);
  virtual ~FluxBC();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  virtual RealGradient computeQpFluxResidual() = 0;
  virtual RealGradient computeQpFluxJacobian() = 0;
};

#endif /* FLUXBC_H */
