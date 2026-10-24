//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVSP3TemperatureSourceSink.h"
#include "MathUtils.h"

registerMooseObject("HeatTransferApp", FVSP3TemperatureSourceSink);

InputParameters
FVSP3TemperatureSourceSink::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription("Computes residual for the derived temperature source in the SP3 "
                             "thermal radiation transport model.");

  params.addParam<std::vector<MooseFunctorName>>(
      "absorptivities", "The vector of absorptivities of the energy bands considered.");
  params.addParam<std::vector<MooseFunctorName>>(
      "psi_1", "The vector of radiation flux moments from group 1.");
  params.addParam<std::vector<MooseFunctorName>>(
      "psi_2", "The vector of radiation flux moments from group 2");

  params.set<bool>("force_boundary_execution") = true;
  params.set<unsigned short>("ghost_layers") = 2;

  params.suppressParameter<bool>("force_boundary_execution");

  return params;
}

FVSP3TemperatureSourceSink::FVSP3TemperatureSourceSink(const InputParameters & params)
  : FVFluxKernel(params),
    _absorptivity_vec(getParam<std::vector<MooseFunctorName>>("absorptivities")),
    _psi1_vec(getParam<std::vector<MooseFunctorName>>("psi_1")),
    _psi2_vec(getParam<std::vector<MooseFunctorName>>("psi_2"))
{

  unsigned int num_values = _psi1_vec.size();

  // Reading in psi1 - first order of the thermal radiation flux moment
  _psi1_vec_functors.resize(num_values);
  for (const auto i : make_range(num_values))
    _psi1_vec_functors[i] = &getFunctor<ADReal>(_psi1_vec[i]);

  // Reading in psi2 - second order of the thermal radiation flux moment
  if (_psi2_vec.size() != num_values)
    mooseError("Number of variables in the psi_2 vector is not the same than the psi_1 vector!");
  _psi2_vec_functors.resize(num_values);
  for (const auto i : make_range(num_values))
    _psi2_vec_functors[i] = &getFunctor<ADReal>(_psi2_vec[i]);

  // Reading in the absorptivities
  _absorptivity_vec_functors.resize(num_values);
  if (_absorptivity_vec.size() == 1)
    for (const auto i : make_range(num_values))
      _absorptivity_vec_functors[i] = &getFunctor<ADReal>(_absorptivity_vec[0]);
  else if (_absorptivity_vec.size() == num_values)
    for (const auto i : make_range(num_values))
      _absorptivity_vec_functors[i] = &getFunctor<ADReal>(_absorptivity_vec[i]);
  else
    mooseError(
        "Number of variables in the absorptivities vector is not the same than the psi_1 vector!");
}

ADReal
FVSP3TemperatureSourceSink::computeQpResidual()
{
  const auto N = _psi1_vec_functors.size();

  using namespace Moose::FV;
  const auto state = determineState();
  const auto face_arg =
      Moose::FaceArg({_face_info, LimiterType::CentralDifference, true, false, nullptr, nullptr});

  ADReal thermal_flux(0.0);
  for (std::size_t index = 0; index < N; ++index)
  {
    const auto band_grad_phi_1 =
        _a1 * _psi1_vec_functors[index]->gradient(face_arg, state) * _face_info->normal();
    const auto band_grad_phi_2 =
        _a2 * _psi2_vec_functors[index]->gradient(face_arg, state) * _face_info->normal();
    const auto band_kappa = (*_absorptivity_vec_functors[index])(face_arg, state);

    thermal_flux += 1. / band_kappa * (band_grad_phi_1 + band_grad_phi_2);
  }

  return -1 * thermal_flux;
}
