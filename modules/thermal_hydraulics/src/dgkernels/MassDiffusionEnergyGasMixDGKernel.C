//
// This file is part of Sockeye heat pipe performance code.
// All rights reserved; see COPYRIGHT for full restrictions.
//

#include "MassDiffusionEnergyGasMixDGKernel.h"
#include "VaporMixtureFluidProperties.h"
#include "IdealGasFluidProperties.h"
#include "FlowModelGasMixUtils.h"
#include "MooseUtils.h"

registerMooseObject("ThermalHydraulicsApp", MassDiffusionEnergyGasMixDGKernel);

InputParameters
MassDiffusionEnergyGasMixDGKernel::validParams()
{
  InputParameters params = MassDiffusionBaseGasMixDGKernel::validParams();

  params.addRequiredParam<MaterialPropertyName>("pressure", "Mixture pressure material property");
  params.addRequiredParam<MaterialPropertyName>("temperature",
                                                "Mixture temperature material property");
  params.addRequiredParam<MaterialPropertyName>("velocity", "Mixture velocity material property");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The VaporMixtureFluidProperties object");

  params.addClassDescription("Adds mass diffusion to the energy equation for FlowChannelGasMix.");

  return params;
}

MassDiffusionEnergyGasMixDGKernel::MassDiffusionEnergyGasMixDGKernel(
    const InputParameters & parameters)
  : MassDiffusionBaseGasMixDGKernel(parameters),

    _p_elem(getADMaterialProperty<Real>("pressure")),
    _p_neig(getNeighborADMaterialProperty<Real>("pressure")),
    _T_elem(getADMaterialProperty<Real>("temperature")),
    _T_neig(getNeighborADMaterialProperty<Real>("temperature")),
    _vel_elem(getADMaterialProperty<Real>("velocity")),
    _vel_neig(getNeighborADMaterialProperty<Real>("velocity")),

    _fp(getUserObject<VaporMixtureFluidProperties>("fluid_properties")),
    _fp_primary(_fp.getPrimaryFluidProperties()),
    _fp_secondary(_fp.getSecondaryFluidProperties())
{
  if (!dynamic_cast<const IdealGasFluidProperties *>(&_fp_primary) ||
      !dynamic_cast<const IdealGasFluidProperties *>(&_fp_secondary))
    mooseError("This class requires gases to use IdealGasFluidProperties.");
}

ADReal
MassDiffusionEnergyGasMixDGKernel::computeQpFlux() const
{
  Real dx, dx_side;
  computePositionChanges(dx, dx_side);

  const ADReal rho = linearlyInterpolate(_rho_elem[_qp], _rho_neig[_qp], dx, dx_side);
  const ADReal D = linearlyInterpolate(_D_elem[_qp], _D_neig[_qp], dx, dx_side);
  const ADReal dxi_dx = computeGradient(_mass_fraction_elem[_qp], _mass_fraction_neig[_qp], dx);

  const ADReal mass_flux_secondary = -rho * D * dxi_dx;
  const ADReal mass_flux_primary = -mass_flux_secondary;

  const ADReal xi_secondary =
      linearlyInterpolate(_mass_fraction_elem[_qp], _mass_fraction_neig[_qp], dx, dx_side);
  const ADReal xi_primary = 1 - xi_secondary;

  const ADReal phi_secondary =
      FlowModelGasMixUtils::computeSecondaryMoleFraction<true>(xi_secondary, _fp);
  const ADReal phi_primary = 1 - phi_secondary;

  const ADReal p_mix = linearlyInterpolate(_p_elem[_qp], _p_neig[_qp], dx, dx_side);
  const ADReal p_primary = phi_primary * p_mix;
  const ADReal p_secondary = phi_secondary * p_mix;

  const ADReal T = linearlyInterpolate(_T_elem[_qp], _T_neig[_qp], dx, dx_side);

  const ADReal vel_mix = linearlyInterpolate(_vel_elem[_qp], _vel_neig[_qp], dx, dx_side);
  const ADReal diffvel_primary = MooseUtils::absoluteFuzzyEqual(xi_primary, 0.0)
                                     ? 0.0
                                     : mass_flux_primary / (rho * xi_primary);
  const ADReal diffvel_secondary = MooseUtils::absoluteFuzzyEqual(xi_secondary, 0.0)
                                       ? 0.0
                                       : mass_flux_secondary / (rho * xi_secondary);
  const ADReal vel_primary = vel_mix + diffvel_primary;
  const ADReal vel_secondary = vel_mix + diffvel_secondary;

  const ADReal H_primary = computeComponentTotalEnthalpy(p_primary, T, vel_primary, _fp_primary);
  const ADReal H_secondary =
      computeComponentTotalEnthalpy(p_secondary, T, vel_secondary, _fp_secondary);

  return mass_flux_primary * H_primary + mass_flux_secondary * H_secondary;
}

ADReal
MassDiffusionEnergyGasMixDGKernel::computeComponentTotalEnthalpy(
    const ADReal & p,
    const ADReal & T,
    const ADReal & vel,
    const SinglePhaseFluidProperties & fp) const
{
  const ADReal e = fp.e_from_p_T(p, T);
  const ADReal rho = fp.rho_from_p_T(p, T);

  // Ideal gas assumption is used here to eliminate pressure in numerator and denominator,
  // which when zero, would cause NaN
  const ADReal h = e + FluidProperties::_R * T / fp.molarMass();

  return h + 0.5 * vel * vel;
}
