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

#ifndef DARCYPRESSURE_H
#define DARCYPRESSURE_H

#include "Diffusion.h"

class DarcyPressure;

template<>
InputParameters validParams<DarcyPressure>();


class DarcyPressure : public Diffusion
{
public:
  DarcyPressure(const std::string & name, InputParameters parameters);
  virtual ~DarcyPressure();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _permeability;
  Real _viscosity;
};


#endif /* DARCYPRESSURE_H */
