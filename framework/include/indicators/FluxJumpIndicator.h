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

#ifndef FLUXJUMPINDICATOR_H
#define FLUXJUMPINDICATOR_H

#include "JumpIndicator.h"

class FluxJumpIndicator;

template<>
InputParameters validParams<FluxJumpIndicator>();

class FluxJumpIndicator :
  public JumpIndicator
{
public:
  FluxJumpIndicator(const std::string & name, InputParameters parameters);
  virtual ~FluxJumpIndicator(){};

protected:

  virtual Real computeQpIntegral();

  std::string _property_name;
  MaterialProperty<Real> & _property;
  MaterialProperty<Real> & _property_neighbor;
};

#endif /* FLUXJUMPINDICATOR_H */
