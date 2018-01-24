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
#include "TypesMaterial.h"
#include "libmesh/dense_matrix.h"

template <>
InputParameters
validParams<TypesMaterial>()
{
  InputParameters params = validParams<Material>();
  return params;
}

TypesMaterial::TypesMaterial(const InputParameters & parameters)
  : Material(parameters),
    _real_prop(declareProperty<Real>("real_prop")),
    _std_vec_prop(declareProperty<std::vector<Real>>("stdvec_prop")),
    _std_vec_prop_qp(declareProperty<std::vector<Real>>("stdvec_prop_qp")),
    _std_vec_grad_prop(declareProperty<std::vector<RealGradient>>("stdvec_grad_prop")),
    _real_vec_prop(declareProperty<RealVectorValue>("realvec_prop")),
    _matrix_prop(declareProperty<DenseMatrix<Real>>("matrix_prop")),
    _tensor_prop(declareProperty<RealTensorValue>("tensor_prop"))
{
}

void
TypesMaterial::computeQpProperties()
{
  _real_prop[_qp] = 1;

  _std_vec_prop[_qp].resize(2);
  _std_vec_prop[_qp][0] = 9;
  _std_vec_prop[_qp][1] = 8;

  _std_vec_prop_qp[_qp].resize(1);
  _std_vec_prop_qp[_qp][0] = 1.;
  if (_qp == 0)
    _std_vec_prop_qp[_qp][0] = 0.;

  _std_vec_grad_prop[_qp].resize(2);
  _std_vec_grad_prop[_qp][0](0) = 2;
  _std_vec_grad_prop[_qp][0](1) = 5;
  _std_vec_grad_prop[_qp][0](2) = 7;
  _std_vec_grad_prop[_qp][1](0) = 10;
  _std_vec_grad_prop[_qp][1](1) = 12;
  _std_vec_grad_prop[_qp][1](2) = 15;

  _real_vec_prop[_qp](0) = 6;
  _real_vec_prop[_qp](1) = 5;
  _real_vec_prop[_qp](2) = 4;

  _matrix_prop[_qp].resize(1, 2);
  _matrix_prop[_qp](0, 0) = 20;
  _matrix_prop[_qp](0, 1) = 21;

  _tensor_prop[_qp](0, 0) = 11;
  _tensor_prop[_qp](1, 1) = 22;
  _tensor_prop[_qp](2, 2) = 33;
}
