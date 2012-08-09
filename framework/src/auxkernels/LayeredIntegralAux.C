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

#include "LayeredIntegralAux.h"
#include "LayeredIntegral.h"

template<>
InputParameters validParams<LayeredIntegralAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("layered_integral", "The LayeredIntegral UserObject to get values from.");
  return params;
}

LayeredIntegralAux::LayeredIntegralAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _layered_integral(getUserObject<LayeredIntegral>("layered_integral"))
{
}

Real
LayeredIntegralAux::computeValue()
{
  return _layered_integral.value(_current_elem->centroid());
}
