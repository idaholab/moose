//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NewmarkBetaContact.h"
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "ConstraintWarehouse.h"
#include "Constraint.h"
#include "KernelBase.h"
#include "DGKernelBase.h"
#include "InterfaceKernelBase.h"
#include "IntegratedBCBase.h"

#include "libmesh/numeric_vector.h"

registerMooseObject("MooseApp", NewmarkBetaContact);

InputParameters
NewmarkBetaContact::validParams()
{
  InputParameters params = NewmarkBeta::validParams();
  params.addParam<std::vector<std::string>>(
      "displacements", "The variables corresponding to the x y z displacements of the mesh.");
  return params;
}

NewmarkBetaContact::NewmarkBetaContact(const InputParameters & parameters)
  : NewmarkBeta(parameters),
    _disp(getParam<std::vector<std::string>>("displacements").begin(),
          getParam<std::vector<std::string>>("displacements").end()),
    _contact_tag_id(_fe_problem.addVectorTag("contact")),
    _contact_residual(_nl.addVector(_contact_tag_id, false, PARALLEL)),
    _noncontact_tag_id(_fe_problem.addVectorTag("noncontact")),
    _noncontact_residual(_nl.addVector(_noncontact_tag_id, false, PARALLEL)),
    _implicit_u_fraction(2 * _beta),
    _implicit_udot_fraction(_gamma),
    _noncontact_old(_nl.addVector("noncontact_old", false, PARALLEL))
{
  _fe_problem.setUDotDotRequested(false);
  _fe_problem.setUDotDotOldRequested(false);
}

void
NewmarkBetaContact::postResidual(NumericVector<Number> & residual)
{
  // time derivative residuals for all variables
  residual += _Re_time;
  // non-time residuals for all non-displacement variables
  residual += _Re_non_time;
  // contact residuals for displacement variables
  residual += _contact_residual;
  // noncontact residuals for displacement variables
  residual.add(_implicit_u_fraction, _noncontact_residual);
  residual.add(1 - _implicit_u_fraction, _noncontact_old);

  // residual corresponding to -2 * u_dot_old / dt. Note that u_dot(_old) *does not* have units of
  // m/s. Rather it has units of kg * m / s per its computation in postStep
  const NumericVector<Number> & u_dot_old = *_sys.solutionUDotOld();
  residual.add(-2. / _dt, u_dot_old);

  residual.close();
}

template <typename T>
void
NewmarkBetaContact::changeNonContactTags(T & warehouse)
{
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    auto & residual_objects = warehouse.getObjects(tid);
    std::for_each(residual_objects.begin(), residual_objects.end(), [this](auto & ro) {
      if (_disp.find(ro->variable().name()) != _disp.end())
      {
        auto * const nc_ro =
            const_cast<ResidualObject *>(static_cast<const ResidualObject *>(ro.get()));
        if (nc_ro->hasVectorTag("nontime"))
        {
          nc_ro->useVectorTag("noncontact");
          nc_ro->eraseVectorTag("nontime");
          nc_ro->assignMatrixCoeff("system", _implicit_u_fraction);
        }
      }
    });
  }
}

void
NewmarkBetaContact::init()
{
  auto & constraints = _nl.getConstraintWarehouse().getObjects(/*tid = 0*/);
  std::for_each(constraints.begin(), constraints.end(), [this](auto & constraint) {
    if (_disp.find(constraint->variable().name()) != _disp.end())
    {
      auto * const nc_constraint = const_cast<Constraint *>(constraint.get());
      nc_constraint->eraseVectorTag("nontime");
      nc_constraint->useVectorTag("contact");
    }
  });

  changeNonContactTags(_nl.getKernelWarehouse());
  changeNonContactTags(_nl.getDGKernelWarehouse());
  changeNonContactTags(_nl.getInterfaceKernelWarehouse());
  changeNonContactTags(_nl.getIntegratedBCWarehouse());

  // // compute residual for the initial time step
  // _nl.computeResidualTag(_nl.RHS(), _noncontact_tag_id);
  // _noncontact_old = _nl.RHS();
}

void
NewmarkBetaContact::postStep()
{
  // Compute "current" time derivative and then shift. What is used in residual evaluation is
  // u_dot_old
  NumericVector<Number> & u_dot_old = *_sys.solutionUDotOld();
  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  u_dot = u_dot_old;
  u_dot.add(-_dt * _implicit_udot_fraction, _noncontact_residual);
  u_dot.add(-_dt * (1. - _implicit_udot_fraction), _noncontact_old);
  u_dot.add(-_dt, _contact_residual);
  u_dot_old = u_dot;

  // now shift noncontact residual
  _noncontact_old = _noncontact_residual;
}

void
NewmarkBetaContact::computeTimeDerivatives()
{
  if (!_sys.solutionUDot())
    mooseError("NewmarkBeta: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase befor requesting `u_dot`.");

  if (!_sys.solutionUDotOld())
    mooseError("NewmarkBeta: Old time derivative of solution (`u_dot_old`) is not stored. Please "
               "set uDotOldRequested() to true in FEProblemBase befor requesting `u_dot_old`.");

  // Users should not be using the current time derivatives in their residual objects if they're
  // using this time integrator, but this method always gets called so we can't just error. We could
  // insert a bunch of NaNs into the u_dot vector to try and catch bad use of u_dot, but for now I'm
  // just going to return
  return;
}

void
NewmarkBetaContact::computeADTimeDerivatives(ADReal &, const dof_id_type &, ADReal &) const
{
  mooseError("If using NewmarkBetaContact, your residual objects should not involve current time "
             "derivatives");
}
