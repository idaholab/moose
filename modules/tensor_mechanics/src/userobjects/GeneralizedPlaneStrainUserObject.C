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
  params.addParam<FunctionName>("traction_zz", "0", "Function used to prescribe traction in the out-of-plane direction");
  params.addParam<Real>("factor", 1.0, "Scale factor applied to prescribed traction");
  params.addParam<std::string>("base_name", "Material properties base name");
  params.set<bool>("use_displaced_mesh") = true;
  params.set<MultiMooseEnum>("execute_on") = "linear";

  return params;
}

GeneralizedPlaneStrainUserObject::GeneralizedPlaneStrainUserObject(const InputParameters & parameters) :
  ElementUserObject(parameters),
  _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
  _Cijkl(getMaterialProperty<RankFourTensor>(_base_name + "elasticity_tensor")),
  _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
  _traction_zz(getFunction("traction_zz")),
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
    _residual += _JxW[_qp] * _coord[_qp] * (_stress[_qp](2, 2) - _traction_zz.value(_t, _q_point[_qp]) * _factor);

  // diagonal jacobian, integral of C(2, 2, 2, 2)
  for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
    _jacobian += _JxW[_qp] * _coord[_qp] * _Cijkl[_qp](2, 2, 2, 2);
}

void
GeneralizedPlaneStrainUserObject::threadJoin(const UserObject & uo)
{
  const GeneralizedPlaneStrainUserObject & gpsuo = static_cast<const GeneralizedPlaneStrainUserObject &>(uo);
  _residual += gpsuo._residual;
  _jacobian += gpsuo._jacobian;
}

void
GeneralizedPlaneStrainUserObject::finalize()
{
  gatherSum(_residual);
  gatherSum(_jacobian);
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
