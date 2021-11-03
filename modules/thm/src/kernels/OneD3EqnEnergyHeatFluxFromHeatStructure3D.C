#include "OneD3EqnEnergyHeatFluxFromHeatStructure3D.h"
#include "HeatFluxFromHeatStructureBaseUserObject.h"
#include "THMIndices3Eqn.h"
#include "FlowModelSinglePhase.h"
#include "HeatConductionModel.h"

registerMooseObject("THMApp", OneD3EqnEnergyHeatFluxFromHeatStructure3D);

InputParameters
OneD3EqnEnergyHeatFluxFromHeatStructure3D::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<UserObjectName>("user_object",
                                          "Layered average wall temperature user object");
  params.addRequiredParam<MaterialPropertyName>("T", "Fluid temperature");
  params.addRequiredParam<MaterialPropertyName>("Hw", "Convective heat transfer coefficient");
  params.addRequiredCoupledVar("P_hf", "Heat flux perimeter");
  params.addRequiredCoupledVar("rhoA", "rho*A of the flow channel");
  params.addRequiredCoupledVar("rhouA", "rhou*A of the flow channel");
  params.addRequiredCoupledVar("rhoEA", "rhoE*A of the flow channel");
  return params;
}

OneD3EqnEnergyHeatFluxFromHeatStructure3D::OneD3EqnEnergyHeatFluxFromHeatStructure3D(
    const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _user_object(getUserObjectBase("user_object")),
    _Hw(getMaterialProperty<Real>("Hw")),
    _T(getMaterialProperty<Real>("T")),
    _dT_drhoA(getMaterialPropertyDerivativeTHM<Real>("T", "rhoA")),
    _dT_drhouA(getMaterialPropertyDerivativeTHM<Real>("T", "rhouA")),
    _dT_drhoEA(getMaterialPropertyDerivativeTHM<Real>("T", "rhoEA")),
    _P_hf(coupledValue("P_hf")),
    _rhoA_jvar(coupled("rhoA")),
    _rhouA_jvar(coupled("rhouA")),
    _rhoEA_jvar(coupled("rhoEA"))
{
}

Real
OneD3EqnEnergyHeatFluxFromHeatStructure3D::computeQpResidual()
{
  const Real T_wall = _user_object.spatialValue(_current_elem->vertex_average());
  return _Hw[_qp] * (_T[_qp] - T_wall) * _P_hf[_qp] * _test[_i][_qp];
}

Real
OneD3EqnEnergyHeatFluxFromHeatStructure3D::computeQpJacobian()
{
  return _Hw[_qp] * _P_hf[_qp] * _dT_drhoEA[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}

Real
OneD3EqnEnergyHeatFluxFromHeatStructure3D::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_jvar)
    return _Hw[_qp] * _P_hf[_qp] * _dT_drhoA[_qp] * _phi[_j][_qp] * _test[_i][_qp];

  else if (jvar == _rhouA_jvar)
    return _Hw[_qp] * _P_hf[_qp] * _dT_drhouA[_qp] * _phi[_j][_qp] * _test[_i][_qp];

  else
    return 0.;
}
