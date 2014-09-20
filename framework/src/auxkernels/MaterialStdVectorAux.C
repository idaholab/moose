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
  InputParameters params = validParams<MaterialAuxBase<std::vector<Real> > >();
  params.addParam<unsigned int>("index", 0, "The index to consider for this kernel");
  params.addClassDescription("Extracts a component of a material's std::vector<Real> to an aux variable.  If the std::vector is not of sufficient size then zero is returned");

  return params;
}

MaterialStdVectorAux::MaterialStdVectorAux(const std::string & name, InputParameters parameters) :
  MaterialAuxBase<std::vector<Real> >(name, parameters),
    _index(getParam<unsigned int>("index"))
{
}

MaterialStdVectorAux::~MaterialStdVectorAux()
{
}

Real
MaterialStdVectorAux::computeValue()
{
  mooseAssert(_prop[_qp].size() > _index, "MaterialStdVectorAux: You chose to extract component " << _index << " but your Material property only has size " << _prop[_qp].size());
  return _factor * _prop[_qp][_index] + _offset;
}
