#include "OneD7EqnMomentumVolumeFractionGradientNoPhaseInteraction.h"

registerMooseObject("THMApp", OneD7EqnMomentumVolumeFractionGradientNoPhaseInteraction);

template <>
InputParameters
validParams<OneD7EqnMomentumVolumeFractionGradientNoPhaseInteraction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("arhoA", "The density of the kth phase");
  params.addRequiredCoupledVar("arhouA", "The momentum of the kth phase");
  params.addRequiredCoupledVar("arhoEA", "The total energy of the kth phase");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredCoupledVar("alpha", "The volume fraction of the kth phase");
  params.addRequiredParam<MaterialPropertyName>("direction",
                                                "The direction of the pipe material property");
  params.addRequiredParam<MaterialPropertyName>("p", "Pressure");
  return params;
}

OneD7EqnMomentumVolumeFractionGradientNoPhaseInteraction::
    OneD7EqnMomentumVolumeFractionGradientNoPhaseInteraction(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _area(coupledValue("A")),
    _alpha_grad(coupledGradient("alpha")),

    _dir(getMaterialProperty<RealVectorValue>("direction")),

    _p(getMaterialProperty<Real>("p")),
    _dp_darhoA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeTHM<Real>("p", "arhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoEA")),

    _arhoA_var_number(coupled("arhoA")),
    _arhoEA_var_number(coupled("arhoEA"))
{
}

Real
OneD7EqnMomentumVolumeFractionGradientNoPhaseInteraction::computeQpResidual()
{
  return -_p[_qp] * _area[_qp] * _alpha_grad[_qp] * _dir[_qp] * _test[_i][_qp];
}

Real
OneD7EqnMomentumVolumeFractionGradientNoPhaseInteraction::computeQpJacobian()
{
  return -_dp_darhouA[_qp] * _area[_qp] * _alpha_grad[_qp] * _dir[_qp] * _phi[_j][_qp] *
         _test[_i][_qp];
}

Real
OneD7EqnMomentumVolumeFractionGradientNoPhaseInteraction::computeQpOffDiagJacobian(
    unsigned int jvar)
{
  if (jvar == _arhoA_var_number)
  {
    return -_dp_darhoA[_qp] * _area[_qp] * _alpha_grad[_qp] * _dir[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];
  }
  else if (jvar == _arhoEA_var_number)
  {
    return -_dp_darhoEA[_qp] * _area[_qp] * _alpha_grad[_qp] * _dir[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];
  }
  else
    return 0;
}
