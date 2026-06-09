#include "ReducedPressurePIMPLE.h"

#include "FEProblem.h"
#include "AuxiliarySystem.h"
#include "LinearSystem.h"

#include <cmath>

using namespace libMesh;

registerMooseObject("NavierStokesApp", ReducedPressurePIMPLE);

InputParameters
ReducedPressurePIMPLE::validParams()
{
  InputParameters params = PIMPLE::validParams();
  params += ReducedPressurePIMPLESolve::validParams();
  params.addClassDescription(
      "Solves the transient Navier-Stokes equations using a reduced-pressure PIMPLE algorithm with "
      "sharp-interface face-flux hooks and linear finite volume variables.");
  return params;
}

ReducedPressurePIMPLE::ReducedPressurePIMPLE(const InputParameters & parameters)
  : PIMPLE(parameters), _reduced_pimple_solve(*this)
{
  _fixed_point_solve->setInnerSolve(_reduced_pimple_solve);
}

void
ReducedPressurePIMPLE::init()
{
  _reduced_pimple_solve.initialSetup();
  TransientBase::init();
  _reduced_pimple_solve.linkRhieChowUserObject();
  _reduced_pimple_solve.setupPressurePin();
}

void
ReducedPressurePIMPLE::takeStep(Real input_dt)
{
  Real dt_to_take = input_dt;
  if (dt_to_take == -1.0)
    dt_to_take = computeConstrainedDT();

  if (_reduced_pimple_solve.adjustMomentumPressureTimeStepEnabled() && dt_to_take > 0.0)
  {
    const Real courant = _reduced_pimple_solve.momentumPressureCourant(dt_to_take);
    const auto courant_audit = _reduced_pimple_solve.momentumPressureCourantAudit(dt_to_take);
    const Real required_dt = _reduced_pimple_solve.constrainedMomentumPressureDT(dt_to_take);
    const Real adjusted_dt = std::max(required_dt, _dtmin);

    if (std::isfinite(courant) && adjusted_dt > 0.0 && adjusted_dt < dt_to_take)
    {
      _console << name() << ": reducing dt from " << dt_to_take << " to " << adjusted_dt
               << " to keep momentum/pressure CFL <= "
               << _reduced_pimple_solve.momentumPressureMaxCourant() << " (current CFL="
               << courant;
      if (required_dt < _dtmin)
        _console << ", requested dt " << required_dt << " is below dtmin=" << _dtmin;
      if (courant_audit.has_worst_cell)
      {
        _console << ", worst cell id=" << courant_audit.worst_cell_id
                 << " centroid=" << courant_audit.worst_cell_centroid
                 << " volume=" << courant_audit.worst_cell_volume
                 << " flux_sum=" << courant_audit.worst_cell_flux_sum;
        if (courant_audit.has_worst_face)
          _console << ", worst face id=" << courant_audit.worst_face_id
                   << " face_centroid=" << courant_audit.worst_face_centroid
                   << " face_normal=" << courant_audit.worst_face_normal
                   << " face_phi=" << courant_audit.worst_face_volumetric_flux
                   << " face_flux=" << courant_audit.worst_face_integrated_flux;
      }
      _console << ")" << std::endl;
      dt_to_take = adjusted_dt;
    }
  }

  TransientBase::takeStep(dt_to_take);
  _reduced_pimple_solve.commitAcceptedTimestepTransportHistory();
}

Real
ReducedPressurePIMPLE::relativeSolutionDifferenceNorm(bool check_aux) const
{
  if (check_aux)
    return _aux.solution().l2_norm_diff(_aux.solutionOld()) / _aux.solution().l2_norm();
  else
  {
    Real residual = 0;
    for (const auto sys : _reduced_pimple_solve.systemsToSolve())
      residual +=
          std::pow(sys->solution().l2_norm_diff(sys->solutionOld()) / sys->solution().l2_norm(), 2);
    return std::sqrt(residual);
  }
}

std::set<TimeIntegrator *>
ReducedPressurePIMPLE::getTimeIntegrators() const
{
  std::set<TimeIntegrator *> tis;
  for (const auto sys : _reduced_pimple_solve.systemsToSolve())
    for (const auto & ti : sys->getTimeIntegrators())
      tis.insert(ti.get());
  return tis;
}
