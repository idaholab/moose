#include "OneDEnergyHRhoUBC.h"

registerMooseObject("THMApp", OneDEnergyHRhoUBC);

template <>
InputParameters
validParams<OneDEnergyHRhoUBC>()
{
  InputParameters params = validParams<OneDIntegratedBC>();
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction");
  params.addRequiredParam<Real>("H", "Specified enthalpy");
  params.addRequiredParam<Real>("rhou", "Specified momentum");
  params.addRequiredCoupledVar("A", "Area");
  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");

  params.declareControllable("H rhou");

  return params;
}

OneDEnergyHRhoUBC::OneDEnergyHRhoUBC(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("alpha", "beta")
                                    : nullptr),
    _H(getParam<Real>("H")),
    _rhou(getParam<Real>("rhou")),
    _area(coupledValue("A")),
    _beta_var_num(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint)
{
}

Real
OneDEnergyHRhoUBC::computeQpResidual()
{
  return _alpha[_qp] * _rhou * _H * _area[_qp] * _normal * _test[_i][_qp];
}

Real
OneDEnergyHRhoUBC::computeQpJacobian()
{
  return 0;
}

Real
OneDEnergyHRhoUBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _beta_var_num)
  {
    return (*_dalpha_dbeta)[_qp] * _rhou * _H * _area[_qp] * _normal * _phi[_j][_qp] *
           _test[_i][_qp];
  }
  else
    return 0;
}
