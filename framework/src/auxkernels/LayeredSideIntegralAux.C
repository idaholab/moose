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

#include "LayeredSideIntegralAux.h"
#include "LayeredSideIntegral.h"

template<>
InputParameters validParams<LayeredSideIntegralAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("layered_integral", "The LayeredSideIntegral UserObject to get values from.");
  return params;
}

LayeredSideIntegralAux::LayeredSideIntegralAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _layered_integral(getUserObject<LayeredSideIntegral>("layered_integral"))
{
}

Real
LayeredSideIntegralAux::computeValue()
{
  return _layered_integral.integralValue(_current_elem->centroid());
}
