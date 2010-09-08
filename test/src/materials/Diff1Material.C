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

#include "Diff1Material.h"
#include "dense_matrix.h"

template<>
InputParameters validParams<Diff1Material>()
{
  InputParameters params = validParams<Material>();
  params.set<Real>("diff") = 1.0;
  return params;
}

Diff1Material::Diff1Material(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  : Material(name, moose_system, parameters),
    _diff(getParam<Real>("diff")),
    _diffusivity(declareProperty<Real>("diff1")),
    _vprop(declareProperty<std::vector<Real> >("vprop")),
    _matrix_mat(declareProperty<DenseMatrix<Real> >("matrix_mat"))
{
}

void
Diff1Material::computeProperties()
{
  for (unsigned int qp = 0; qp < _n_qpoints; qp++)
    _diffusivity[qp] = _diff;
}
