//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeSharpInterfaceRhieChowMassFlux.h"

#include <cmath>

registerMooseObject("NavierStokesApp", ConservativeSharpInterfaceRhieChowMassFlux);

InputParameters
ConservativeSharpInterfaceRhieChowMassFlux::validParams()
{
  InputParameters params = ConservativeSharpInterfaceRhieChowMassFluxBase::validParams();
  params.set<MooseEnum>("hydrostatic_predictor_discretization") = "discrete_density_sn_grad";
  params.addClassDescription(
      "Sharp-interface Rhie-Chow face-flux provider for velocity-component momentum. The primary "
      "unknowns are U components; density weighting enters through the "
      "momentum operator and mass-flux functors.");
  return params;
}

ConservativeSharpInterfaceRhieChowMassFlux::ConservativeSharpInterfaceRhieChowMassFlux(
    const InputParameters & params)
  : ConservativeSharpInterfaceRhieChowMassFluxBase(params),
    _continuity_error(_fe_problem.mesh(), blockIDs(), "conservative_continuity_error", false)
{
  for (const auto tid : make_range(libMesh::n_threads()))
    UserObject::_subproblem.addFunctor("conservative_continuity_error", _continuity_error, tid);
}

void
ConservativeSharpInterfaceRhieChowMassFlux::addMomentumPredictorExplicitForcing(
    const unsigned int system_i, NumericVector<Number> & rhs) const
{
  mooseAssert(system_i < _momentum_implicit_systems.size(),
              "Momentum component index out of range.");

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const auto & dof_indices = elem_info->dofIndices()[_global_momentum_system_numbers[system_i]];
    if (dof_indices.empty())
      continue;

    const Real force_density =
        reducedPressureMomentumPredictorForceDensity(*elem_info, Moose::currentState())(system_i);
    rhs.add(dof_indices[0], force_density * elem_info->volume() * elem_info->coordFactor());
  }
  rhs.close();
}

void
ConservativeSharpInterfaceRhieChowMassFlux::updateContinuityErrorField()
{
  _continuity_error.clear();

  const Real dt = _fe_problem.dt();
  const bool use_time_derivative = std::abs(dt) > libMesh::TOLERANCE;

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const auto elem_arg = makeElemArg(elem_info->elem());
    Real cont_err = 0.0;
    if (use_time_derivative)
      cont_err += (_rho(elem_arg, Moose::currentState()) - _rho(elem_arg, Moose::oldState())) / dt;

    _continuity_error[elem_info->elem()->id()] = cont_err;
  }

  for (const auto * fi : flowFaceInfo())
  {
    const auto flux_it = _face_mass_flux.find(fi->id());
    if (flux_it == _face_mass_flux.end())
      continue;

    const Real rho_phi = flux_it->second;
    const Real face_measure = fi->faceArea() * fi->faceCoord();

    if (fi->elemPtr() && hasBlocks(fi->elemPtr()->subdomain_id()))
    {
      const auto & elem_info = *fi->elemInfo();
      const Real cell_volume = elem_info.volume() * elem_info.coordFactor();
      if (cell_volume > libMesh::TOLERANCE)
        _continuity_error[elem_info.elem()->id()] += rho_phi * face_measure / cell_volume;
    }

    if (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id()))
    {
      const auto & neighbor_info = *fi->neighborInfo();
      const Real cell_volume = neighbor_info.volume() * neighbor_info.coordFactor();
      if (cell_volume > libMesh::TOLERANCE)
        _continuity_error[neighbor_info.elem()->id()] -= rho_phi * face_measure / cell_volume;
    }
  }
}
