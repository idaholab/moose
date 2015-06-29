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

#include "MaterialStdVectorAux.h"

template<>
InputParameters validParams<MaterialStdVectorAux>()
{
  InputParameters params = validParams<MaterialStdVectorAuxBase<> >();
  params.addClassDescription("Extracts a component of a material type std::vector<Real> to an aux variable.  If the std::vector is not of sufficient size then zero is returned");
  return params;
}

MaterialStdVectorAux::MaterialStdVectorAux(const std::string & name, InputParameters parameters) :
  MaterialStdVectorAuxBase<Real>(name, parameters)
{
}

MaterialStdVectorAux::~MaterialStdVectorAux()
{
}

Real
MaterialStdVectorAux::getRealValue()
{
  return _prop[_qp][_index];
}
