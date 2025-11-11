//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumericalFlux3EqnInternalValues.h"
#include "THMIndicesVACE.h"
#include "ADNumericalFlux3EqnBase.h"

registerMooseObject("ThermalHydraulicsApp", NumericalFlux3EqnInternalValues);

InputParameters
NumericalFlux3EqnInternalValues::validParams()
{
  InputParameters params = InternalSideVectorPostprocessor::validParams();
  params += SamplerBase::validParams();
  params.addRequiredCoupledVar("A_linear", "Cross-sectional area, linear");
  params.addRequiredParam<UserObjectName>("numerical_flux", "Name of numerical flux user object");
  params.addClassDescription("Computes internal fluxes for FlowChannel1Phase.");
  return params;
}

NumericalFlux3EqnInternalValues::NumericalFlux3EqnInternalValues(const InputParameters & parameters)
  : InternalSideVectorPostprocessor(parameters),
    SamplerBase(parameters, this, _communicator),
    _A1(adCoupledValue("A_linear")),
    _A2(adCoupledNeighborValue("A_linear")),
    _rhoA1(getADMaterialProperty<Real>("rhoA")),
    _rhouA1(getADMaterialProperty<Real>("rhouA")),
    _rhoEA1(getADMaterialProperty<Real>("rhoEA")),
    _rhoA2(getNeighborADMaterialProperty<Real>("rhoA")),
    _rhouA2(getNeighborADMaterialProperty<Real>("rhouA")),
    _rhoEA2(getNeighborADMaterialProperty<Real>("rhoEA")),
    _numerical_flux(getUserObject<ADNumericalFlux3EqnBase>("numerical_flux"))
{
  std::vector<std::string> var_names(THMVACE1D::N_FLUX_OUTPUTS);
  var_names[THMVACE1D::MASS] = "mass_flux";
  var_names[THMVACE1D::MOMENTUM] = "momentum_flux";
  var_names[THMVACE1D::ENERGY] = "energy_flux";

  SamplerBase::setupVariables(var_names);
}

void
NumericalFlux3EqnInternalValues::initialize()
{
  SamplerBase::initialize();
}

void
NumericalFlux3EqnInternalValues::execute()
{
  // Assume we are in 1D, and internal sides have only a single quadrature point
  const unsigned int _qp = 0;

  std::vector<ADReal> U1 = {_rhoA1[_qp], _rhouA1[_qp], _rhoEA1[_qp], _A1[_qp]};
  std::vector<ADReal> U2 = {_rhoA2[_qp], _rhouA2[_qp], _rhoEA2[_qp], _A2[_qp]};

  const Real nLR_dot_d = _current_side * 2 - 1.0;

  const std::vector<ADReal> & flux_elem_ad =
      _numerical_flux.getFlux(_current_side, _current_elem->id(), true, U1, U2, nLR_dot_d);
  const std::vector<ADReal> & flux_neig_ad =
      _numerical_flux.getFlux(_current_side, _current_elem->id(), false, U1, U2, nLR_dot_d);

  // Convert vector to non-AD
  std::vector<Real> flux(flux_elem_ad.size());
  for (const auto i : index_range(flux))
    flux[i] = MetaPhysicL::raw_value(0.5 * (flux_elem_ad[i] + flux_neig_ad[i]));

  SamplerBase::addSample(_q_point[_qp], _current_elem->id(), flux);
}

void
NumericalFlux3EqnInternalValues::finalize()
{
  SamplerBase::finalize();
}

void
NumericalFlux3EqnInternalValues::threadJoin(const UserObject & y)
{
  const auto & vpp = static_cast<const NumericalFlux3EqnInternalValues &>(y);

  SamplerBase::threadJoin(vpp);
}
