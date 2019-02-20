#include "WallHeatTransferCoefficient3EqnHeliumMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("THMApp", WallHeatTransferCoefficient3EqnHeliumMaterial);

template <>
InputParameters
validParams<WallHeatTransferCoefficient3EqnHeliumMaterial>()
{
  InputParameters params = validParams<WallHeatTransferCoefficient3EqnBaseMaterial>();
  return params;
}

WallHeatTransferCoefficient3EqnHeliumMaterial::WallHeatTransferCoefficient3EqnHeliumMaterial(
    const InputParameters & parameters)
  : WallHeatTransferCoefficient3EqnBaseMaterial(parameters),
    _T_wall(getMaterialProperty<Real>("T_wall"))
{
}

void
WallHeatTransferCoefficient3EqnHeliumMaterial::computeQpProperties()
{
  Real cp = _fp.cp_from_v_e(_v[_qp], _e[_qp]);
  Real mu = _fp.mu_from_v_e(_v[_qp], _e[_qp]);
  Real k = _fp.k_from_v_e(_v[_qp], _e[_qp]);

  Real Re = THM::Reynolds(1., _rho[_qp], _vel[_qp], _D_h[_qp], mu);
  Real Pr = THM::Prandtl(cp, mu, k);
  Real Nu = 0.021 * std::pow(Re, 0.8) * std::pow(Pr, 0.4) * std::pow(_T_wall[_qp] / _T[_qp], -0.5);
  _Hw[_qp] = THM::wallHeatTransferCoefficient(Nu, k, _D_h[_qp]);
}
