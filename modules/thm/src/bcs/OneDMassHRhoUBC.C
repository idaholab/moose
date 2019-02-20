#include "OneDMassHRhoUBC.h"

registerMooseObject("THMApp", OneDMassHRhoUBC);

template <>
InputParameters
validParams<OneDMassHRhoUBC>()
{
  InputParameters params = validParams<OneDIntegratedBC>();
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction");
  params.addRequiredParam<Real>("rhou", "Specified momentum");
  params.addRequiredCoupledVar("A", "Area");
  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");

  params.declareControllable("rhou");

  return params;
}

OneDMassHRhoUBC::OneDMassHRhoUBC(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("alpha", "beta")
                                    : nullptr),
    _rhou(getParam<Real>("rhou")),
    _area(coupledValue("A")),
    _beta_var_num(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint)
{
}

Real
OneDMassHRhoUBC::computeQpResidual()
{
  return _alpha[_qp] * _rhou * _area[_qp] * _normal * _test[_i][_qp];
}

Real
OneDMassHRhoUBC::computeQpJacobian()
{
  return 0;
}

Real
OneDMassHRhoUBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _beta_var_num)
  {
    return (*_dalpha_dbeta)[_qp] * _area[_qp] * _rhou * _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
