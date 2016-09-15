/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GeneralizedPlaneStrainUO.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<GeneralizedPlaneStrainUO>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription("Generalized Plane Strain UserObject to provide Residual and diagonal Jacobian entry");
  params.addParam<FunctionName>("function", "0", "Function used to prescribe pressure in the out-of-plane direction");
  params.addParam<Real>("factor", 1.0, "Scale factor applied to prescribed pressure");
  params.set<bool>("use_displaced_mesh") = true;
  params.set<MultiMooseEnum>("execute_on") = "linear";

  return params;
}

GeneralizedPlaneStrainUO::GeneralizedPlaneStrainUO(const InputParameters & parameters) :
  ElementUserObject(parameters),
  _Cijkl(getMaterialProperty<RankFourTensor>("elasticity_tensor")),
  _stress(getMaterialProperty<RankTwoTensor>("stress")),
  _function(getFunction("function")),
  _factor(getParam<Real>("factor"))
{
}

void
GeneralizedPlaneStrainUO::initialize()
{
  _residual = 0;
  _jacobian = 0;
}

void
GeneralizedPlaneStrainUO::execute()
{
  // residual, integral of stress_zz
  for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
    _residual += _JxW[_qp] * _coord[_qp] * (_stress[_qp](2, 2) - _function.value(_t, _q_point[_qp]) * _factor);

  // diagonal jacobian, integral of C(2, 2, 2, 2)
  for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
    _jacobian += _JxW[_qp] * _coord[_qp] * _Cijkl[_qp](2, 2, 2, 2);
}

Real
GeneralizedPlaneStrainUO::returnResidual() const
{
  return _residual;
}

Real
GeneralizedPlaneStrainUO::returnJacobian() const
{
  return _jacobian;
}
