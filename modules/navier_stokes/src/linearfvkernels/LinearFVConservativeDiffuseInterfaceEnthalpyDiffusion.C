//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion.h"
#include "LinearFVConservativeDiffuseInterfaceEnthalpyBC.h"
#include "ConservativeDiffuseInterfaceMixtureEnthalpy.h"
#include "MooseLinearVariableFV.h"
#include "NSFVUtils.h"

registerMooseObject("NavierStokesApp", LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion);

InputParameters
LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Heat conduction in the equation for the conserved mixture enthalpy "
                             "density of a diffuse-interface flow.");
  params.addRequiredParam<UserObjectName>(
      "mixture_enthalpy", "The ConservativeDiffuseInterfaceMixtureEnthalpy relating rho*h and T.");
  // Harmonic interpolation is the series resistance of the two half cells, which conducts exactly
  // across a face between cells of different conductivity
  MooseEnum conductivity_interpolation("average harmonic", "harmonic");
  params.addParam<MooseEnum>("conductivity_interpolation",
                             conductivity_interpolation,
                             "Interpolation of the cell conductivities to an internal face.");
  return params;
}

LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion::
    LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion(const InputParameters & params)
  : LinearFVFluxKernel(params),
    _mixture(getUserObject<ConservativeDiffuseInterfaceMixtureEnthalpy>("mixture_enthalpy")),
    _conductivity_interp_method(getParam<MooseEnum>("conductivity_interpolation") == "harmonic"
                                    ? Moose::FV::InterpMethod::HarmonicAverage
                                    : Moose::FV::InterpMethod::Average),
    _elem_coeff(0.0),
    _neighbor_coeff(0.0),
    _flux_constant(0.0)
{
}

void
LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion::initialSetup()
{
  for (const auto & [_, bc] : _var.getBoundaryConditionMap())
    if (!dynamic_cast<const LinearFVConservativeDiffuseInterfaceEnthalpyBC *>(bc))
      mooseError(bc->type(), " is not a compatible boundary condition with ", type(), "!");
}

void
LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion::setupFaceData(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setupFaceData(face_info);

  _elem_coeff = 0.0;
  _neighbor_coeff = 0.0;
  _flux_constant = 0.0;

  if (_current_face_type != FaceInfo::VarFaceNeighbors::BOTH)
    return;

  const auto elem_lin = _mixture.cellLinearization(*face_info->elemInfo());
  const auto neighbor_lin = _mixture.cellLinearization(*face_info->neighborInfo());
  Real face_conductivity;
  Moose::FV::interpolate(_conductivity_interp_method,
                         face_conductivity,
                         elem_lin.conductivity,
                         neighbor_lin.conductivity,
                         *face_info,
                         true);
  const Real conductance = face_conductivity / face_info->dCNMag();

  // -k (T_N - T_P) / d with T = rho*h / C + offset in each cell
  _elem_coeff = conductance / elem_lin.heat_capacity;
  _neighbor_coeff = -conductance / neighbor_lin.heat_capacity;
  _flux_constant =
      conductance * (ConservativeDiffuseInterfaceMixtureEnthalpy::temperatureOffset(elem_lin) -
                     ConservativeDiffuseInterfaceMixtureEnthalpy::temperatureOffset(neighbor_lin));
}

Real
LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion::computeBoundaryMatrixContribution(
    const LinearFVBoundaryCondition & bc)
{
  computeBoundaryRHSContribution(bc);
  return _elem_coeff * _current_face_area;
}

Real
LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion::computeBoundaryRHSContribution(
    const LinearFVBoundaryCondition & bc)
{
  if (!_cached_rhs_contribution)
  {
    const auto & enthalpy_bc =
        static_cast<const LinearFVConservativeDiffuseInterfaceEnthalpyBC &>(bc);
    if (enthalpy_bc.hasTemperature())
    {
      const auto & cell = NS::linearFVFaceSideElemInfo(*_current_face_info, _current_face_type);
      const auto lin = _mixture.cellLinearization(cell);
      const Real conductance = lin.conductivity / enthalpy_bc.cellToFaceDistance();
      // -k (T_b - T_P) / d
      _elem_coeff = conductance / lin.heat_capacity;
      _flux_constant =
          conductance * (ConservativeDiffuseInterfaceMixtureEnthalpy::temperatureOffset(lin) -
                         enthalpy_bc.boundaryTemperature());
    }
    else
      _flux_constant = -enthalpy_bc.heatFlux();

    _cached_rhs_contribution = true;
  }

  return -_flux_constant * _current_face_area;
}
