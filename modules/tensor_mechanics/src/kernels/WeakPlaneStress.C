/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "WeakPlaneStress.h"

#include "Material.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

template <>
InputParameters
validParams<WeakPlaneStress>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Plane stress kernel to provide out-of-plane strain contribution");
  params.addParam<std::string>("base_name", "Material property base name");

  MooseEnum direction("x y z", "z");
  params.addParam<MooseEnum>("direction", direction, "The out of plane direction");

  params.set<bool>("use_displaced_mesh") = false;

  return params;
}

WeakPlaneStress::WeakPlaneStress(const InputParameters & parameters)
  : Kernel(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _Jacobian_mult(getMaterialProperty<RankFourTensor>(_base_name + "Jacobian_mult")),
    _direction(parameters.get<MooseEnum>("direction"))
{
}

Real
WeakPlaneStress::computeQpResidual()
{
  return _stress[_qp](_direction, _direction) * _test[_i][_qp];
}

Real
WeakPlaneStress::computeQpJacobian()
{
  return _Jacobian_mult[_qp](_direction, _direction, _direction, _direction) * _test[_i][_qp] *
         _phi[_j][_qp];
}

Real
WeakPlaneStress::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.0;
}
