/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GeneralizedPlaneStrainUserObject.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "Function.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<GeneralizedPlaneStrainUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription("Generalized Plane Strain UserObject to provide Residual and diagonal Jacobian entry");
  params.addParam<FunctionName>("traction", "0", "Function used to prescribe traction in the out-of-plane direction");
  params.addParam<Real>("factor", 1.0, "Scale factor applied to prescribed traction");
  params.set<bool>("use_displaced_mesh") = true;
  params.set<MultiMooseEnum>("execute_on") = "linear";

  return params;
}

GeneralizedPlaneStrainUserObject::GeneralizedPlaneStrainUserObject(const InputParameters & parameters) :
  ElementUserObject(parameters),
  _Cijkl(getMaterialProperty<RankFourTensor>("elasticity_tensor")),
  _stress(getMaterialProperty<RankTwoTensor>("stress")),
  _traction(getFunction("traction")),
  _factor(getParam<Real>("factor"))
{
}

void
GeneralizedPlaneStrainUserObject::initialize()
{
  _residual = 0;
  _jacobian = 0;
}

void
GeneralizedPlaneStrainUserObject::execute()
{
  // residual, integral of stress_zz
  for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
    _residual += _JxW[_qp] * _coord[_qp] * (_stress[_qp](2, 2) - _traction.value(_t, _q_point[_qp]) * _factor);

  // diagonal jacobian, integral of C(2, 2, 2, 2)
  for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
    _jacobian += _JxW[_qp] * _coord[_qp] * _Cijkl[_qp](2, 2, 2, 2);
}

Real
GeneralizedPlaneStrainUserObject::returnResidual() const
{
  return _residual;
}

Real
GeneralizedPlaneStrainUserObject::returnJacobian() const
{
  return _jacobian;
}
