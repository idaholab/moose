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

#include "PresetBC.h"

template<>
InputParameters validParams<PresetBC>()
{
  InputParameters p = validParams<NodalBC>();
  p.addRequiredParam<Real>("value", "Value of the BC");
  return p;
}


PresetBC::PresetBC(const std::string & name, InputParameters parameters) :
  PresetNodalBC(name, parameters),
  _value(parameters.get<Real>("value"))
{

}

Real
PresetBC::computeQpValue()
{
  return _value;
}
