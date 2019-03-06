//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeFalseMaterial.h"

registerMooseObject("MooseTestApp", ComputeFalseMaterial);

template <>
InputParameters
validParams<ComputeFalseMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription(
      "Simple example of a Material with compute = false.  It has one vector Material property "
      "called 'compute_false_vector', and one Real property called 'compute_false_scalar'.  Both "
      "of these have Old values");
  params.addParam<unsigned>("vector_size", 2, "Size of the vector computed by this class");
  params.set<bool>("compute") = false;
  params.suppressParameter<bool>("compute");
  return params;
}

ComputeFalseMaterial::ComputeFalseMaterial(const InputParameters & parameters)
  : Material(parameters),
    _vector_size(getParam<unsigned>("vector_size")),
    _compute_false_vector(declareProperty<std::vector<Real>>("compute_false_vector")),
    _compute_false_vector_old(getMaterialPropertyOld<std::vector<Real>>("compute_false_vector")),
    _compute_false_scalar(declareProperty<Real>("compute_false_scalar")),
    _compute_false_scalar_old(getMaterialPropertyOld<Real>("compute_false_scalar"))
{
}

void
ComputeFalseMaterial::setQp(unsigned qp)
{
  _qp = qp;
}

void
ComputeFalseMaterial::initQpStatefulProperties()
{
  _compute_false_vector[_qp].assign(_vector_size, 1.0);
  _compute_false_scalar[_qp] = 1.0;
}

void
ComputeFalseMaterial::computeQpThings()
{
  if (_compute_false_vector[_qp].size() != _vector_size)
    mooseError("Size = " + Moose::stringify(_compute_false_vector[_qp].size()) +
               ".  It should be " + Moose::stringify(_vector_size) +
               ".  initQpStatefulProperties did not get called for this element\n");

  for (unsigned i = 0; i < _vector_size; ++i)
    _compute_false_vector[_qp][i] = i + _compute_false_vector_old[_qp][i];
  _compute_false_scalar[_qp] = _compute_false_scalar_old[_qp] * _qp;
}
