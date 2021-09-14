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
#include "NodalBCBase.h"
#include "AuxiliarySystem.h"

#include "libmesh/lumped_mass_matrix.h"

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
    _contact_residual(nullptr),
    _noncontact_tag_id(_fe_problem.addVectorTag("noncontact")),
    _noncontact_residual(nullptr),
    _system_time_tag(_fe_problem.getMatrixTagID("TIME")),
    _implicit_u_fraction(2 * _beta),
    _noncontact_old(nullptr),
    _u_dotdot_contact(nullptr),
    _u_dotdot_internal(nullptr),
    _u_dotdot_internal_old(nullptr)
{
  // Used to create the necessary vectors in the Auxiliary and Nonlinear systems
  _is_newmark_beta_contact = true;
  if (_is_nl_ti)
  {
    _mass_matrix = libmesh_make_unique<LumpedMassMatrix<Number>>(_communicator);
    _mass_matrix->attach_dof_map(_sys.dofMap());
    _mass_matrix_diag = NumericVector<Number>::build(_communicator);
  }
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

  if (_is_nl_ti)
  {
    _mass_matrix->init(PARALLEL);
    const auto & lm_sys = _nl.system();
    _mass_matrix_diag->init(lm_sys.n_dofs(), lm_sys.n_local_dofs(), false, PARALLEL);
  }
}

void
NewmarkBetaContact::postStep()
{
  // now shift noncontact residual
  *_noncontact_old = *_noncontact_residual;
  *_u_dotdot_internal_old = *_u_dotdot_internal;
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

  Moose::out << "u_dot.size() is: " << u_dot.size() << "\n";
  Moose::out << "u_dotdot.size() is: " << u_dotdot.size() << "\n";
  Moose::out << "u_dot_old.size() is: " << u_dot_old.size() << "\n";

  Moose::out << "u_dotdot_old.size() is: " << u_dotdot_old.size() << "\n";
  Moose::out << "_u_dotdot_contact.size() is: " << _u_dotdot_contact->size() << "\n";
  Moose::out << "_u_dotdot_internal.size() is: " << _u_dotdot_internal->size() << "\n";
  Moose::out << "_u_dotdot_internal_old.size() is: " << _u_dotdot_internal_old->size() << "\n";
  Moose::out << "_contact_residual.size() is: " << _contact_residual->size() << "\n";

  if (_fe_problem.timeStep() <= _inactive_tsteps)
  {
    u_dot.zero();
    u_dotdot.zero();
  }
  else
  {
    u_dotdot = *_solution;
    computeTimeDerivativeHelper(u_dot,
                                _solution_old,
                                u_dot_old,
                                u_dotdot,
                                u_dotdot_old,
                                *_u_dotdot_contact,
                                *_u_dotdot_internal,
                                *_u_dotdot_internal_old);
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
NewmarkBetaContact::addTimeIntegratorVectors(const bool is_nonlinear_system)
{
  mooseAssert(is_nonlinear_system == _is_nl_ti, "These should be consistent");
  if (is_nonlinear_system)
  {
    _contact_residual = &_nl.addVector(_contact_tag_id, true, GHOSTED);
    _noncontact_residual = &_nl.addVector(_noncontact_tag_id, true, GHOSTED);
    _noncontact_old = &_nl.addVector("noncontact_old", true, GHOSTED);
    _u_dotdot_contact = &_nl.addVector("u_dotdot_contact", true, GHOSTED);
    _u_dotdot_internal = &_nl.addVector("_u_dotdot_internal", true, GHOSTED);
    _u_dotdot_internal_old = &_nl.addVector("_u_dotdot_internal_old", true, GHOSTED);
  }
  else
  {
    // I don't think we should be adding anything residual related to the aux system ti
    _contact_residual = &_fe_problem.getAuxiliarySystem().addVector(_contact_tag_id, true, GHOSTED);
    _noncontact_residual =
        &_fe_problem.getAuxiliarySystem().addVector(_noncontact_tag_id, true, GHOSTED);
    _noncontact_old = &_fe_problem.getAuxiliarySystem().addVector("noncontact_old", true, GHOSTED);

    _u_dotdot_contact =
        &_fe_problem.getAuxiliarySystem().addVector("u_dotdot_contact", true, GHOSTED);
    _u_dotdot_internal =
        &_fe_problem.getAuxiliarySystem().addVector("_u_dotdot_internal", true, GHOSTED);
    _u_dotdot_internal_old =
        &_fe_problem.getAuxiliarySystem().addVector("_u_dotdot_internal_old", true, GHOSTED);
  }
}

void
NewmarkBetaContact::computeContactAccelerations()
{
  if (!_is_nl_ti)
    // I don't think we should do anything residual related in the aux ti
    return;

  Moose::out << "After calling get_system_matrix \n";

  // Ok nodal boundary conditions are marked by default as contributing to the 'time' matrix tag
  // which makes sense ... when performing an explicit solve you still must make sure you're
  // satisfying your boundary conditions. However, we just want to compute a mass matrix so we need
  // to make sure that nodal bcs are not overwriting our Jacobian and messing with our mass
  auto & nbc_warehouse = _nl.getNodalBCWarehouse();
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    auto & nbcs = nbc_warehouse.getObjects(tid);
    std::for_each(nbcs.begin(), nbcs.end(), [this](auto & nbc) { nbc->eraseMatrixTag("time"); });
  }

  _nl.associateMatrixToTag(*_mass_matrix, _nl.systemMatrixTag());
  _fe_problem.computeJacobianTag(
      *_nonlinear_implicit_system->current_local_solution, *_mass_matrix, _system_time_tag);
  _nl.disassociateMatrixFromTag(*_mass_matrix, _nl.systemMatrixTag());

  // Now add back the time tag. (Question to self: is this actually necessary? Is the 'time' matrix
  // tag only relevant to fully explicit solves?)
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    auto & nbcs = nbc_warehouse.getObjects(tid);
    std::for_each(nbcs.begin(), nbcs.end(), [this](auto & nbc) { nbc->useMatrixTag("time"); });
  }

  // The matrix should be assembled at the end of computeJacobianTag, so we don't have to close
  // again here
  mooseAssert(_mass_matrix->closed(), "We should have assembled the mass matrix");
  _mass_matrix->get_diagonal(*_mass_matrix_diag);

  for (const auto i :
       make_range(_mass_matrix_diag->first_local_index(), _mass_matrix_diag->last_local_index()))
    if ((*_mass_matrix_diag)(i) == 0)
      _mass_matrix_diag->set(i, 1);

  // After adding or setting we must assemble
  _mass_matrix_diag->close();

  // "Invert" the diagonal mass matrix
  _mass_matrix_diag->reciprocal();

  // Multiply the inversion by the RHS
  _u_dotdot_contact->pointwise_mult(*_mass_matrix_diag, *_contact_residual);

  // Check for convergence by seeing if there is a nan or inf
  auto sum = _u_dotdot_contact->sum();
  bool converged = std::isfinite(sum);

  // Get things back to normal after mass matrix manipulations.
  // FIXME: Not sure how this is consistent for auxiliary system
  // I don't know why this is here
  // _nl.computeJacobian(*_mass_matrix);

  Moose::out << "Can we lump the mass matrix correctly in the integrator? " << converged << "\n";
}

void
NewmarkBetaContact::computeADTimeDerivatives(ADReal & /*ad_u_dot*/,
                                             const dof_id_type & /*dof*/,
                                             ADReal & /*ad_u_dotdot*/) const
{
  // mooseError("Let's take care of AD later");

  //  const auto & u_old = _solution_old(dof);
  //  const auto & u_dot_old = (*_sys.solutionUDotOld())(dof);
  //  const auto & u_dotdot_old = (*_sys.solutionUDotDotOld())(dof);

  // Seeds ad_u_dotdot with _ad_dof_values and associated derivatives provided via ad_u_dot from
  // MooseVariableData
  //  ad_u_dotdot = ad_u_dot;

  //  computeTimeDerivativeHelper(ad_u_dot,
  //                              _solution_old,
  //                              u_dot_old,
  //                              ad_u_dotdot,
  //                              u_dotdot_old,
  //                              *_u_dotdot_contact,
  //                              *_u_dotdot_internal,
  //                              *_u_dotdot_internal_old);

  /*
  computeTimeDerivativeHelper(
      ad_u_dot, u_old, u_dot_old, ad_u_dotdot, u_dotdot_old, ad_u_dotdot, ad_u_dotdot, ad_u_dotdot);
*/
}

void
NewmarkBetaContact::postResidual(NumericVector<Number> & residual)
{
  // time derivative residuals for all variables
  residual += _Re_time;
  // non-time residuals for all non-displacement variables excluding contact
  residual += *_noncontact_residual;
  // contact residuals for displacement variables
  residual.add(1.0 / _implicit_u_fraction, *_contact_residual);

  // Compute contact accelerations. We're about to compute a Jacobian so we need to make sure we
  // toggle our AD system on, otherwise bad things will happen like too many derivative insertions
  ADReal::do_derivatives = true;
  computeContactAccelerations();
  ADReal::do_derivatives = false;

  // compute _u_dotdot_contact and cache
  residual.close();
}
