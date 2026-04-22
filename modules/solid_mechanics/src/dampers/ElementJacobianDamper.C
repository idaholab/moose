//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementJacobianDamper.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "DisplacedProblem.h"
#include "Assembly.h"

#include "libmesh/quadrature.h" // _qrule

// C++
#include <algorithm>
#include <limits>

registerMooseObject("SolidMechanicsApp", ElementJacobianDamper);

namespace
{
bool
isRecoverableProbeException(const std::string & message)
{
  return message.find("Jacobian") != std::string::npos ||
         message.find("singular") != std::string::npos ||
         message.find("det != 0") != std::string::npos;
}

void
restoreNodes(Elem & elem, const std::vector<Point> & point_copies)
{
  mooseAssert(elem.n_nodes() == point_copies.size(), "Node restore cache is the wrong size");

  for (unsigned int i = 0; i < elem.n_nodes(); ++i)
    elem.node_ref(i) = point_copies[i];
}
}

InputParameters
ElementJacobianDamper::validParams()
{
  InputParameters params = GeneralDamper::validParams();
  params.addClassDescription("Damper that limits the change in element Jacobians");
  params.addParam<std::vector<VariableName>>(
      "displacements", {}, "The nonlinear displacement variables");
  params.addParam<Real>(
      "max_increment",
      0.1,
      "The maximum permissible relative increment in the Jacobian per Newton iteration");
  params.addParam<bool>(
      "use_backtracking",
      false,
      "If true, iteratively cut back the probed Newton update until the displaced mesh remains "
      "nondegenerate and the Jacobian increment stays within max_increment.");
  params.addRangeCheckedParam<Real>(
      "backtrack_factor",
      0.5,
      "backtrack_factor > 0 & backtrack_factor < 1",
      "Multiplicative cutback applied to the trial damping during ElementJacobianDamper "
      "backtracking.");
  params.addParam<unsigned int>(
      "max_backtrack_steps", 12, "Maximum number of ElementJacobianDamper backtracking attempts.");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

ElementJacobianDamper::ElementJacobianDamper(const InputParameters & parameters)
  : GeneralDamper(parameters),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid, _sys.number())),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _fe_problem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _displaced_problem(_fe_problem.getDisplacedProblem()),
    _max_jacobian_diff(parameters.get<Real>("max_increment")),
    _use_backtracking(getParam<bool>("use_backtracking")),
    _backtrack_factor(getParam<Real>("backtrack_factor")),
    _max_backtrack_steps(getParam<unsigned int>("max_backtrack_steps"))
{
  if (_displaced_problem == NULL)
    mooseError("ElementJacobianDamper: Must use displaced problem");

  _mesh = &_displaced_problem->mesh();

  const std::vector<VariableName> & nl_vnames(getParam<std::vector<VariableName>>("displacements"));
  _ndisp = nl_vnames.size();

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp_var.push_back(&_sys.getFieldVariable<Real>(_tid, nl_vnames[i]));
    _disp_incr.push_back(_disp_var.back()->increment());
  }
}

void
ElementJacobianDamper::initialSetup()
{
}

Real
ElementJacobianDamper::computeDamping(const NumericVector<Number> & /* solution */,
                                      const NumericVector<Number> & update)
{
  auto probe_trial =
      [&](const Real trial_damping, Real & max_difference, std::string & error_message)
  {
    const bool valid_local = probeDamping(update, trial_damping, max_difference, error_message);

    _communicator.max(max_difference);

    int invalid_local = valid_local ? 0 : 1;
    std::vector<int> invalid_ranks(_communicator.size());
    _communicator.allgather(invalid_local, invalid_ranks);

    processor_id_type invalid_processor = 0;
    const auto invalid_it = std::find(invalid_ranks.begin(), invalid_ranks.end(), 1);
    const bool invalid_global = invalid_it != invalid_ranks.end();
    if (invalid_global)
      invalid_processor = std::distance(invalid_ranks.begin(), invalid_it);

    if (invalid_global)
      _communicator.broadcast(error_message, invalid_processor);

    return !invalid_global;
  };

  if (!_use_backtracking)
  {
    Real max_difference = 0.0;
    std::string error_message;

    PARALLEL_TRY
    {
      if (!probe_trial(/*trial_damping=*/1.0, max_difference, error_message))
        _fe_problem.setException(error_message);
    }
    PARALLEL_CATCH;

    if (max_difference > _max_jacobian_diff)
      return _max_jacobian_diff / max_difference;

    return 1.0;
  }

  const Real minimum_trial_damping = std::max(_min_damping, std::numeric_limits<Real>::epsilon());
  Real damping = 1.0;
  std::string error_message;

  for (unsigned int step = 0; step <= _max_backtrack_steps; ++step)
  {
    Real max_difference = 0.0;
    bool valid = false;

    PARALLEL_TRY { valid = probe_trial(damping, max_difference, error_message); }
    PARALLEL_CATCH;

    unsigned int has_exception = _fe_problem.hasException() ? 1 : 0;
    _communicator.max(has_exception);
    if (has_exception)
      return damping;

    if (valid && max_difference <= _max_jacobian_diff)
      return damping;

    if (damping <= minimum_trial_damping || step == _max_backtrack_steps)
    {
      if (!valid)
        _fe_problem.setException(error_message.empty()
                                     ? "ElementJacobianDamper could not find a nondegenerate "
                                       "trial update."
                                     : error_message);
      else
        _fe_problem.setException("ElementJacobianDamper could not reduce the relative Jacobian "
                                 "increment below max_increment without driving the damping "
                                 "factor below a usable threshold.");
      return damping;
    }

    if (!valid)
      damping *= _backtrack_factor;
    else
      damping =
          std::min(damping * _backtrack_factor, damping * _max_jacobian_diff / max_difference);
  }

  return damping;
}

bool
ElementJacobianDamper::probeDamping(const NumericVector<Number> & update,
                                    const Real damping,
                                    Real & max_difference,
                                    std::string & error_message)
{
  MooseArray<Real> JxW_displaced;

  // Vector for storing the original node coordinates
  std::vector<Point> point_copies;

  max_difference = 0.0;
  error_message.clear();

  try
  {
    // Loop over elements in the mesh
    for (auto & current_elem : _mesh->getMesh().active_local_element_ptr_range())
    {
      point_copies.clear();
      point_copies.reserve(current_elem->n_nodes());

      // Displace nodes with the trial Newton increment
      for (unsigned int i = 0; i < current_elem->n_nodes(); ++i)
      {
        Node & displaced_node = current_elem->node_ref(i);

        point_copies.push_back(displaced_node);

        for (unsigned int j = 0; j < _ndisp; ++j)
        {
          const dof_id_type disp_dof_num =
              displaced_node.dof_number(_sys.number(), _disp_var[j]->number(), 0);
          displaced_node(j) += damping * update(disp_dof_num);
        }
      }

      try
      {
        // Reinit element to compute Jacobian of the trial displaced element
        _assembly.reinit(current_elem);
        JxW_displaced = _JxW;
      }
      catch (const std::exception & e)
      {
        restoreNodes(*current_elem, point_copies);

        // Degenerate-map failures mean this trial damping is too aggressive
        if (isRecoverableProbeException(e.what()))
        {
          error_message = e.what();
          return false;
        }

        throw;
      }

      restoreNodes(*current_elem, point_copies);

      // Reinit element to compute Jacobian before displacement
      try
      {
        _assembly.reinit(current_elem);
      }
      catch (const std::exception & e)
      {
        if (isRecoverableProbeException(e.what()))
        {
          error_message = e.what();
          return false;
        }

        throw;
      }

      for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
      {
        const Real denominator = std::max(std::abs(_JxW[qp]), libMesh::TOLERANCE);
        const Real diff = std::abs(JxW_displaced[qp] - _JxW[qp]) / denominator;
        if (diff > max_difference)
          max_difference = diff;
      }

      JxW_displaced.release();
    }
  }
  catch (const MooseException & e)
  {
    error_message = e.what();

    if (isRecoverableProbeException(error_message))
      return false;

    _fe_problem.setException(error_message);
    return false;
  }

  return true;
}
