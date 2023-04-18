//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UELThread.h"
#include "NonlinearSystemBase.h"

UELThread::UELThread(FEProblemBase & fe_problem, AbaqusUserElement & uel_uo)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _sys(fe_problem.currentNonlinearSystem()),
    _variables(uel_uo.getVariables()),
    _uel_uo(uel_uo),
    _uel(uel_uo.getPlugin()),
    _statev_copy(uel_uo._nstatev)
{
  // regular step
  _lflags[3] = 0;

  // newton based solution (should be 1 when using a predictor or maybe at the start of the
  // iteration)
  _lflags[3] = 0;
}

// Splitting Constructor
UELThread::UELThread(UELThread & x, Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split),
    _sys(x._sys),
    _variables(x._variables),
    _uel_uo(x._uel_uo),
    _uel(x._uel),
    _lflags(x._lflags),
    _statev_copy(_uel_uo._nstatev)
{
}

void
UELThread::subdomainChanged()
{
}

/**
 * Fortran array memory layout: (1,1), (2,1) (3,1) (1,2) (2,2) (3,2) (1,3) (2,3) (3,3)
 * C++ array memory layout:     [0][0], [0][1], [0][2], [1][0], [1][1], [1][2], [2][0], [2][1],
 * [2][2]
 */

void
UELThread::onElement(const Elem * elem)
{
  // get all dof indices for the coupled variables on this element
  int nnode = elem->n_nodes();
  const auto nvar = _variables.size();
  _all_dof_indices.resize(nnode * nvar);

  // prepare DOF indices in the correct order
  for (const auto i : index_range(_variables))
  {
    _variables[i]->getDofIndices(elem, _var_dof_indices);

    // sanity checks
    if (_variables[i]->scalingFactor() != 1)
      mooseError("Scaling factors other than unity are not yet supported");
    if (static_cast<int>(_var_dof_indices.size()) != nnode)
      mooseError("All coupled variables must be full order lagrangian");

    for (const auto j : make_range(nnode))
      _all_dof_indices[j * nvar + i] = _var_dof_indices[j];
  }

  int ndofel = _all_dof_indices.size();

  // get solution values
  _all_dof_values.resize(ndofel);
  _sys.currentSolution()->get(_all_dof_indices, _all_dof_increments);
  _all_dof_increments.resize(ndofel);
  _sys.solutionOld().get(_all_dof_indices, _all_dof_values);
  mooseAssert(_all_dof_values.size() == _all_dof_increments.size(), "Inconsistent solution size.");
  for (const auto i : index_range(_all_dof_values))
    _all_dof_increments[i] -= _all_dof_values[i];

  _all_udot_dof_values.resize(ndofel);
  _all_udotdot_dof_values.resize(ndofel);

  // get u_dot and u_dotdot solution values
  if (false)
  {
    _all_udot_dof_values.resize(ndofel);
    _sys.solutionUDot()->get(_all_dof_indices, _all_udot_dof_values);
    _all_udotdot_dof_values.resize(ndofel);
    _sys.solutionUDot()->get(_all_dof_indices, _all_udotdot_dof_values);
  }

  const bool do_residual = _sys.hasVector(_sys.residualVectorTag());
  const bool do_jacobian = _sys.hasMatrix(_sys.systemMatrixTag());

  // set flags
  if (do_residual && do_jacobian)
    _lflags[2] = 0;
  else if (!do_residual && do_jacobian)
    _lflags[2] = 4;
  else if (do_residual && !do_jacobian)
    _lflags[2] = 5;
  else
    _lflags[2] = 0;

  // copy coords
  int dim = _uel_uo._dim;
  _coords.resize(nnode * dim);
  for (const auto i : make_range(nnode))
    for (const auto j : make_range(dim))
      _coords[j + dim * i] = elem->node_ref(i)(j);

  // prepare residual and jacobian storage
  _local_re.resize(ndofel);
  _local_ke.resize(ndofel, ndofel);

  int nrhs = 1;              // : RHS should contain the residual vector
  int jtype = _uel_uo._jtype; // type of user element
  std::vector<Real> time(2); // dummy
  Real dt = 0;
  std::array<Real, 8> energy;
  int jelem = elem->id() + 1; // User-assigned element number
  Real pnewdt;

  // dummy vars
  Real rdummy = 0;
  int idummy = 0;

  // stateful data
  if (_uel_uo._nstatev)
  {
    const auto & statev_old = _uel_uo._statev[_uel_uo._statev_index_old][elem->id()];
    std::copy(statev_old.begin(), statev_old.end(), _statev_copy.begin());
  }

  // call the plugin
  _uel(_local_re.get_values().data(),
       _local_ke.get_values().data(),
       _statev_copy.data(),
       energy.data(),
       &ndofel,
       &nrhs,
       &_uel_uo._nstatev,
       _uel_uo._props.data(),
       &_uel_uo._nprops,
       _coords.data(),
       &dim,
       &nnode,
       _all_dof_values.data() /* U[] */,
       _all_dof_increments.data() /* DU[] */,
       _all_udot_dof_values.data() /* V[] */,
       _all_udotdot_dof_values.data() /* A[] */,
       &jtype,
       time.data(),
       &dt,
       &idummy /* KSTEP */,
       &idummy /* KINC */,
       &jelem,
       nullptr /* PARAMS[] */,
       &idummy /* NDLOAD */,
       nullptr /* JDLTYP[] */,
       nullptr /* ADLMAG[] */,
       nullptr /* PREDEF[] */,
       &idummy /* NPREDF */,
       nullptr /* LFLAGS[] */,
       &ndofel /* MLVARX */,
       nullptr /* DDLMAG[] */,
       &idummy /* MDLOAD */,
       &pnewdt,
       nullptr /* JPROPS[] */,
       &idummy /* NJPROP */,
       &rdummy /* PERIOD */
  );

  if (_uel_uo._nstatev)
  {
    auto & statev_current = _uel_uo._statev[_uel_uo._statev_index_current][elem->id()];
    std::copy(_statev_copy.begin(), _statev_copy.end(), statev_current.begin());
  }

  // write to the residual vector
  if (do_residual)
    for (const auto tag : _uel_uo.getVectorTags())
      _sys.getVector(tag).add_vector(_local_re, _all_dof_indices);

  // write to the Jacobian (hope we don't have to transpose...)
  if (do_jacobian)
    for (const auto tag : _uel_uo.getMatrixTags())
      _sys.getMatrix(tag).add_matrix(_local_ke, _all_dof_indices);
}
