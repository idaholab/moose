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

#include "Diff2Material.h"

template<>
InputParameters validParams<Diff2Material>()
{
  InputParameters params = validParams<Material>();
  params.set<Real>("diff") = 1.0;
  return params;
}

Diff2Material::Diff2Material(const std::string & name, InputParameters parameters)
  : Material(name, parameters),
    _diff(getParam<Real>("diff")),
    _diffusivity(declareProperty<Real>("diff2")),
    _vector_property(declareProperty<std::vector<Real> >("vector_property"))
{
}

void
Diff2Material::computeProperties()
{
  for (unsigned int qp = 0; qp < _n_qpoints; qp++)
  {
    _diffusivity[qp] = _diff;

    // resize the vector in quadrature point to some random size (10 for instance)
    _vector_property[qp].resize(10);
  }
}
