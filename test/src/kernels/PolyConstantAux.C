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

#include "PolyConstantAux.h"

template<>
InputParameters validParams<PolyConstantAux>()
{
  InputParameters params = validParams<AuxKernel>();
  
  return params;
}

PolyConstantAux::PolyConstantAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters)
{}


Real
PolyConstantAux::computeValue()
{
  Real a = libMesh::pi;
  Real b = 3;
  Real e = 4;
  Real x = (*_current_node)(0);
  Real y = (*_current_node)(1);
  Real z = (*_current_node)(2);
  Real t = _t;
  return a*x*x*x*y*t+b*y*y*z+e*x*y*z*z*z*z;

}
