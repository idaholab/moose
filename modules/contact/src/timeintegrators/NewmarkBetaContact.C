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
    _system_time_tag(_fe_problem.getMatrixTagID("TIME")),
    _mass_matrix_diag(_nl.addVector("mass_matrix_diag", false, PARALLEL)),
    _ones(&_nl.addVector("ones", false, PARALLEL)),
    _implicit_u_fraction(2 * _beta),
    _implicit_udot_fraction(_gamma),
    _noncontact_old(_nl.addVector("noncontact_old", false, PARALLEL)),
    _u_dotdot_contact(_nl.addVector("u_dotdot_contact", false, PARALLEL))
{
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
          // nc_ro->assignMatrixCoeff("system", _implicit_u_fraction);
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

  // now shift noncontact residual
  _noncontact_old = _noncontact_residual;
}

void
NewmarkBetaContact::computeTimeDerivatives()
{
  if (!_sys.solutionUDot())
    mooseError("NewmarkBeta: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase befor requesting `u_dot`.");

  if (!_sys.solutionUDotDot())
    mooseError("NewmarkBeta: Second time derivative of solution (`u_dotdot`) is not stored. Please "
               "set uDotDotRequested() to true in FEProblemBase befor requesting `u_dotdot`.");

  if (!_sys.solutionUDotOld())
    mooseError("NewmarkBeta: Old time derivative of solution (`u_dot_old`) is not stored. Please "
               "set uDotOldRequested() to true in FEProblemBase befor requesting `u_dot_old`.");

  if (!_sys.solutionUDotDotOld())
    mooseError("NewmarkBeta: Old second time derivative of solution (`u_dotdot_old`) is not "
               "stored. Please set uDotDotOldRequested() to true in FEProblemBase befor requesting "
               "`u_dotdot_old`.");

  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  NumericVector<Number> & u_dotdot = *_sys.solutionUDotDot();
  NumericVector<Number> & u_dot_old = *_sys.solutionUDotOld();
  NumericVector<Number> & u_dotdot_old = *_sys.solutionUDotDotOld();

  if (_fe_problem.timeStep() <= _inactive_tsteps)
  {
    u_dot.zero();
    u_dotdot.zero();
  }
  else
  {
    u_dotdot = *_solution;
   computeTimeDerivativeHelper(
        u_dot, _solution_old, u_dot_old, u_dotdot, u_dotdot_old, _u_dotdot_contact);
  }

  // make sure _u_dotdot and _u_dot are in good state
  u_dotdot.close();
  u_dot.close();

  // used for Jacobian calculations
  _du_dotdot_du = 1.0 / _beta / _dt / _dt;
  _du_dot_du = _gamma / _beta / _dt;

  return;
}

void
NewmarkBetaContact::computeContactAccelerations()
{

  auto & mass_matrix = _nonlinear_implicit_system->get_system_matrix();
  Moose::out << "After calling get_system_matrix \n";

  _fe_problem.computeJacobianTag(
      *_nonlinear_implicit_system->current_local_solution, mass_matrix, _system_time_tag);
  ///
  Moose::out << "After calling compute Jacobian \n";
  //
  mass_matrix.vector_mult(_mass_matrix_diag, *_ones);

  // "Invert" the diagonal mass matrix
  _mass_matrix_diag.reciprocal();
  Moose::out << "After taking the reciprocal \n";

  // Multiply the inversion by the RHS
  _u_dotdot_contact.pointwise_mult(_mass_matrix_diag, _contact_residual);

  // Check for convergence by seeing if there is a nan or inf
  auto sum = _u_dotdot_contact.sum();
  bool converged = std::isfinite(sum);


  // Get things back to normal after mass matrix manipulations.
  _nl.computeJacobian(mass_matrix);

  Moose::out << "Can we lump the mass matrix correctly in the integrator? " << converged << "\n";
}

void
NewmarkBetaContact::computeADTimeDerivatives(ADReal & ad_u_dot,
                                             const dof_id_type & dof,
                                             ADReal & ad_u_dotdot) const
{
  mooseError("Let's take care of AD later");

  const auto & u_old = _solution_old(dof);
  const auto & u_dot_old = (*_sys.solutionUDotOld())(dof);
  const auto & u_dotdot_old = (*_sys.solutionUDotDotOld())(dof);

  // Seeds ad_u_dotdot with _ad_dof_values and associated derivatives provided via ad_u_dot from
  // MooseVariableData
  ad_u_dotdot = ad_u_dot;

  computeTimeDerivativeHelper(
      ad_u_dot, u_old, u_dot_old, ad_u_dotdot, u_dotdot_old,_u_dotdot_contact);
}

void
NewmarkBetaContact::postResidual(NumericVector<Number> & residual)
{
  // time derivative residuals for all variables
  residual += _Re_time;
  // non-time residuals for all non-displacement variables excluding contact
  residual += _noncontact_residual;
  // contact residuals for displacement variables
  residual.add(1.0 / _implicit_u_fraction, _contact_residual);

  // Compute contact accelerations
  computeContactAccelerations();
  // compute _u_dotdot_contact and cache
  residual.close();
}
