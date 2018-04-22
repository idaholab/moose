//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GlobalStrainUserObject.h"

#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", GlobalStrainUserObject);

template <>
InputParameters
validParams<GlobalStrainUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription(
      "Global Strain UserObject to provide Residual and diagonal Jacobian entry");
  params.addParam<std::vector<Real>>("applied_stress_tensor",
                                     "Vector of values defining the constant applied stress "
                                     "to add, in order 11, 22, 33, 23, 13, 12");
  params.addParam<Real>("factor", 1.0, "Scale factor applied to prescribed pressure");
  params.addParam<std::string>("base_name", "Material properties base name");
  params.set<ExecFlagEnum>("execute_on") = EXEC_LINEAR;

  return params;
}

GlobalStrainUserObject::GlobalStrainUserObject(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _Cijkl(getMaterialProperty<RankFourTensor>(_base_name + "elasticity_tensor")),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _factor(getParam<Real>("factor"))
{
  if (isParamValid("applied_stress_tensor"))
    _applied_stress_tensor.fillFromInputVector(
        getParam<std::vector<Real>>("applied_stress_tensor"));
  else
    _applied_stress_tensor.zero();
}

void
GlobalStrainUserObject::initialize()
{
  _residual.zero();
  _jacobian.zero();
}

void
GlobalStrainUserObject::execute()
{
  for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // residual, integral of stress components
    _residual += _JxW[_qp] * _coord[_qp] * (_stress[_qp] + _applied_stress_tensor * _factor);

    // diagonal jacobian, integral of elsticity tensor components
    _jacobian += _JxW[_qp] * _coord[_qp] * _Cijkl[_qp];
  }
}

void
GlobalStrainUserObject::threadJoin(const UserObject & uo)
{
  const GlobalStrainUserObject & pstuo = static_cast<const GlobalStrainUserObject &>(uo);

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    {
      _residual(i, j) += pstuo._residual(i, j);

      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
          _jacobian(i, j, k, l) += pstuo._jacobian(i, j, k, l);
    }
}

void
GlobalStrainUserObject::finalize()
{
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    {
      gatherSum(_residual(i, j));

      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
          gatherSum(_jacobian(i, j, k, l));
    }
}

const RankTwoTensor &
GlobalStrainUserObject::getResidual() const
{
  return _residual;
}

const RankFourTensor &
GlobalStrainUserObject::getJacobian() const
{
  return _jacobian;
}
