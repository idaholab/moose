#include "ADOneD3EqnEnergyHeatFluxFromHeatStructure3D.h"
#include "HeatFluxFromHeatStructureBaseUserObject.h"
#include "THMIndices3Eqn.h"
#include "FlowModelSinglePhase.h"
#include "HeatConductionModel.h"

registerMooseObject("ThermalHydraulicsApp", ADOneD3EqnEnergyHeatFluxFromHeatStructure3D);

InputParameters
ADOneD3EqnEnergyHeatFluxFromHeatStructure3D::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredParam<UserObjectName>("user_object",
                                          "Layered average wall temperature user object");
  params.addRequiredParam<MaterialPropertyName>("T", "Fluid temperature");
  params.addRequiredParam<MaterialPropertyName>("Hw", "Convective heat transfer coefficient");
  params.addRequiredCoupledVar("P_hf", "Heat flux perimeter");
  return params;
}

ADOneD3EqnEnergyHeatFluxFromHeatStructure3D::ADOneD3EqnEnergyHeatFluxFromHeatStructure3D(
    const InputParameters & parameters)
  : ADKernel(parameters),
    _user_object(getUserObjectBase("user_object")),
    _Hw(getADMaterialProperty<Real>("Hw")),
    _T(getADMaterialProperty<Real>("T")),
    _P_hf(adCoupledValue("P_hf"))
{
}

ADReal
ADOneD3EqnEnergyHeatFluxFromHeatStructure3D::computeQpResidual()
{
  const Real T_wall = _user_object.spatialValue(_current_elem->vertex_average());
  return _Hw[_qp] * (_T[_qp] - T_wall) * _P_hf[_qp] * _test[_i][_qp];
}
