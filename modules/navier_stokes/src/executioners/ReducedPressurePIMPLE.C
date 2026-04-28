#include "ReducedPressurePIMPLE.h"

#include "FEProblem.h"
#include "AuxiliarySystem.h"
#include "LinearSystem.h"

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
