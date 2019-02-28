#include "FlowRegimeSimpleMaterial.h"
#include "FlowModelTwoPhase.h"

registerMooseObject("THMApp", FlowRegimeSimpleMaterial);

template <>
InputParameters
validParams<FlowRegimeSimpleMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<MaterialPropertyName>("alpha_liquid",
                                                "Volume fraction of the liquid phase");
  params.addRequiredCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  return params;
}

FlowRegimeSimpleMaterial::FlowRegimeSimpleMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),
    _kappa_liquid(declareProperty<Real>(FlowModelTwoPhase::HEAT_FLUX_PARTITIONING_LIQUID)),
    _dkappa_liquid_dbeta(declarePropertyDerivativeTHM<Real>(
        FlowModelTwoPhase::HEAT_FLUX_PARTITIONING_LIQUID, "beta")),

    _alpha_liquid(getMaterialProperty<Real>("alpha_liquid")),
    _dalpha_liquid_dbeta(getMaterialPropertyDerivativeTHM<Real>("alpha_liquid", "beta"))
{
}

void
FlowRegimeSimpleMaterial::computeQpProperties()
{
  _kappa_liquid[_qp] = _alpha_liquid[_qp];
  _dkappa_liquid_dbeta[_qp] = _dalpha_liquid_dbeta[_qp];
}
