//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVKTDC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "Limiter.h"
#include "NavierStokesApp.h"
#include "Executioner.h"

using namespace Moose::FV;

registerMooseObject("NavierStokesApp", PCNSFVKTDC);

InputParameters
PCNSFVKTDC::validParams()
{
  InputParameters params = PCNSFVKT::validParams();
  params.addClassDescription("Computes the residual of advective term using finite volume method "
                             "using a deferred correction approach.");
  params.addParam<Real>(
      "ho_implicit_fraction", 0, "The fraction of the high order flux that should be implicit");
  return params;
}

PCNSFVKTDC::PCNSFVKTDC(const InputParameters & params)
  : PCNSFVKT(params),
    _upwind_limiter(Limiter<ADReal>::build(LimiterType::Upwind)),
    _old_upwind_fluxes(
        declareRestartableData<std::unordered_map<dof_id_type, Real>>("old_upwind_fluxes")),
    _old_ho_fluxes(declareRestartableData<std::unordered_map<dof_id_type, Real>>("old_ho_fluxes")),
    _current_upwind_fluxes(
        declareRestartableData<std::unordered_map<dof_id_type, Real>>("current_upwind_fluxes")),
    _current_ho_fluxes(
        declareRestartableData<std::unordered_map<dof_id_type, Real>>("current_ho_fluxes")),
    _ho_implicit_fraction(getParam<Real>("ho_implicit_fraction"))
{
}

void
PCNSFVKTDC::timestepSetup()
{
  // The SubProblem converged method is not restartable (it relies on data from libMesh) but the
  // Executioner converged method is, so we use that
  if (_app.getExecutioner()->lastSolveConverged())
  {
    _old_upwind_fluxes = _current_upwind_fluxes;
    _old_ho_fluxes = _current_ho_fluxes;
  }
}

void
PCNSFVKTDC::residualSetup()
{
  _current_upwind_fluxes.clear();
  _current_ho_fluxes.clear();
}

void
PCNSFVKTDC::jacobianSetup()
{
  residualSetup();
}

Real
PCNSFVKTDC::getOldFlux(const bool upwind) const
{
  if (_t_step <= 1)
    return 0;

  const auto & flux_container = upwind ? _old_upwind_fluxes : _old_ho_fluxes;
  auto it = flux_container.find(_face_info->id());
  mooseAssert(it != flux_container.end(),
              "We should have saved an old flux for the current _face_info. Do you have mesh "
              "adaptivity on? Unfortunately we don't currently support that");
  return it->second;
}

ADReal
PCNSFVKTDC::computeQpResidual()
{
  mooseAssert(!onBoundary(*_face_info), "We should never execute this object on a boundary");

  const auto current_ho_flux = PCNSFVKT::computeQpResidual();
#ifndef NDEBUG
  auto pr =
#endif
      _current_ho_fluxes.emplace(_face_info->id(), current_ho_flux.value());
  mooseAssert(pr.second,
              "Insertion should have happened. If it did not it means you are overwriting some "
              "other face's flux!");
  _limiter.swap(_upwind_limiter);
  const auto current_upwind_flux = PCNSFVKT::computeQpResidual();
#ifndef NDEBUG
  pr =
#endif
      _current_upwind_fluxes.emplace(_face_info->id(), current_upwind_flux.value());
  mooseAssert(pr.second,
              "Insertion should have happened. If it did not it means you are overwriting some "
              "other face's flux!");
  // Swap back
  _limiter.swap(_upwind_limiter);

  const auto old_upwind_flux = getOldFlux(/*upwind=*/true);
  const auto old_ho_flux = getOldFlux(/*upwind=*/false);

  return _ho_implicit_fraction * current_ho_flux +
         (1. - _ho_implicit_fraction) * (old_ho_flux + current_upwind_flux - old_upwind_flux);
}
