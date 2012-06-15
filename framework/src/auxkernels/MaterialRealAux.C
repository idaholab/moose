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
  params.addRequiredParam<std::string>("matpro", "The material parameter name.");
  return params;
}

MaterialRealAux::MaterialRealAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
  _matpro(getParam<std::string>("matpro")),
  _prop(getMaterialProperty<Real>(_matpro))
{}


Real
MaterialRealAux::computeValue()
{
 return _prop[_qp];
}
