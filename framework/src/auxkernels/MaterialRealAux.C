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

#include "MaterialRealAux.h"

template<>
InputParameters validParams<MaterialRealAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("property", "The scalar material property name");
  params.addParam<Real>("factor", 1, "The factor by which to multiply your material property for visualization");
  params.addParam<Real>("offset", 0, "The offset to add to your material property for visualization");
  return params;
}

MaterialRealAux::MaterialRealAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
  _prop(getMaterialProperty<Real>(getParam<std::string>("property"))),
  _factor(getParam<Real>("factor")),
  _offset(getParam<Real>("offset"))
{}


Real
MaterialRealAux::computeValue()
{
  return _factor * _prop[_qp] + _offset;
}
