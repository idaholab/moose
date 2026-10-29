//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UELThread.h"
#include "NonlinearSystemBase.h"

#include <limits>

UELThread::UELThread(FEProblemBase & fe_problem, AbaqusUserElement & uel_uo)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _sys(fe_problem.currentNonlinearSystem()),
    _aux_sys(&_fe_problem.getAuxiliarySystem()),
    _variables(uel_uo.getVariables()),
    _aux_variables(uel_uo.getAuxVariables()),
    _uel_uo(uel_uo),
    _uel(uel_uo.getPlugin()),
    _lflags{},
    _statev_copy(uel_uo._nstatev),
    _min_pnewdt(std::numeric_limits<Real>::max())
{
  // static analysis
  _lflags[0] = 1;

  // small displacement analysis
  _lflags[1] = 0;

  // general solve step
  _lflags[3] = 0;

  // newton based solution (should be 1 when using a predictor or maybe at the start of the
  // iteration)
  _lflags[4] = 0;
}

// Splitting Constructor
UELThread::UELThread(UELThread & x, Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split),
    _sys(x._sys),
    _aux_sys(x._aux_sys),
    _variables(x._variables),
    _aux_variables(x._aux_variables),
    _uel_uo(x._uel_uo),
    _uel(x._uel),
    _lflags(x._lflags),
    _statev_copy(_uel_uo._nstatev),
    _min_pnewdt(std::numeric_limits<Real>::max())
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

  // ** System variables **
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

  // Get solution values
  _all_dof_values.resize(ndofel);
  _all_dof_increments.resize(ndofel);

  _sys.currentSolution()->get(_all_dof_indices, _all_dof_values);
  _sys.solutionOld().get(_all_dof_indices, _all_dof_increments);

  mooseAssert(_all_dof_values.size() == _all_dof_increments.size(), "Inconsistent solution size.");
  for (const auto i : index_range(_all_dof_values))
    _all_dof_increments[i] = _all_dof_values[i] - _all_dof_increments[i];

  _all_udot_dof_values.resize(ndofel);
  _all_udotdot_dof_values.resize(ndofel);

  // Get u_dot and u_dotdot solution values (required for future expansion of the interface)
  if (false)
  {
    _all_udot_dof_values.resize(ndofel);
    _sys.solutionUDot()->get(_all_dof_indices, _all_udot_dof_values);
    _all_udotdot_dof_values.resize(ndofel);
    _sys.solutionUDot()->get(_all_dof_indices, _all_udotdot_dof_values);
  }

  // Prepare external fields
  const auto nvar_aux = _aux_variables.size();
  _all_aux_var_dof_indices.resize(nnode * nvar_aux);
  _all_aux_var_dof_increments.resize(nnode * nvar_aux);

  for (const auto i : index_range(_aux_variables))
  {
    _aux_variables[i]->getDofIndices(elem, _aux_var_dof_indices);

    if (static_cast<int>(_aux_var_dof_indices.size()) != nnode)
      mooseError("All auxiliary variables must be full order Lagrangian");

    for (const auto j : make_range(nnode))
      _all_aux_var_dof_indices[j * nvar_aux + i] = _aux_var_dof_indices[j];
  }

  _all_aux_var_dof_values.resize(nnode * nvar_aux);
  _aux_var_values_to_uel.resize(nnode * nvar_aux * 2); // Value _and_ increment

  _aux_sys->currentSolution()->get(_all_aux_var_dof_indices, _all_aux_var_dof_values);
  _aux_sys->solutionOld().get(_all_aux_var_dof_indices, _all_aux_var_dof_increments);

  // First, one external field and increment; then, second external field and increment, etc.
  for (const auto i : index_range(_all_aux_var_dof_values))
    _all_aux_var_dof_increments[i] = _all_aux_var_dof_values[i] - _all_aux_var_dof_increments[i];

  for (const auto i : index_range(_all_aux_var_dof_values))
  {
    _aux_var_values_to_uel[i * 2] = _all_aux_var_dof_values[i];
    _aux_var_values_to_uel[i * 2 + 1] = _all_aux_var_dof_increments[i];
  }
  // End of prepare external fields.

  const bool do_residual = _sys.hasVector(_sys.residualVectorTag());
  const bool do_jacobian = _sys.hasMatrix(_sys.systemMatrixTag());

  // set flags
  _lflags[2] = AbaqusUserElement::computeLFlag3(do_residual, do_jacobian);

  // copy coords
  int dim = _uel_uo._dim;
  _coords.resize(nnode * dim);
  for (const auto i : make_range(nnode))
    for (const auto j : make_range(dim))
      _coords[j + dim * i] = elem->node_ref(i)(j);

  // prepare residual and jacobian storage
  _local_re.resize(ndofel);
  _local_ke.resize(ndofel, ndofel);

  Real dt = _fe_problem.dt();
  Real time = _fe_problem.time();
  std::vector<Real> times{time - dt, time - dt}; // first entry should be the step time (TODO)

  // stateful data
  if (_uel_uo._nstatev)
    _statev_copy = libmesh_map_find(_uel_uo._statev[_uel_uo._statev_index_old], elem->id());
  if (_uel_uo._use_energy)
    _energy_copy = libmesh_map_find(_uel_uo._energy_old, elem->id());
  else
    _energy_copy = {};

  // call the plugin
  auto & args = _uel_args;
  args.rhs = _local_re.get_values().data();
  args.amatrx = _local_ke.get_values().data();
  args.svars = _statev_copy.data();
  args.energy = _energy_copy.data();
  args.ndofel = ndofel;
  args.nrhs = 1; // RHS should contain the residual vector
  args.nsvars = _uel_uo._nstatev;
  args.props = _uel_uo._props.data();
  args.nprops = _uel_uo._nprops;
  args.coords = _coords.data();
  args.mcrd = dim;
  args.nnode = nnode;
  args.u = _all_dof_values.data();
  args.du = _all_dof_increments.data();
  args.v = _all_udot_dof_values.data();
  args.a = _all_udotdot_dof_values.data();
  args.jtype = _uel_uo._jtype; // type of user element
  args.time = times.data();
  args.dtime = dt;
  args.kstep = 0;
  args.kinc = 0;
  args.jelem = elem->id() + 1; // User-assigned element number
  args.params = nullptr;
  args.ndload = 0;
  args.jdltyp = nullptr;
  args.adlmag = nullptr;
  args.predef = _aux_var_values_to_uel.data();
  args.npredf = nvar_aux; // Number of external fields.
  args.lflags = _lflags.data();
  args.mlvarx = ndofel;
  args.ddlmag = nullptr;
  args.mdload = 0;
  args.pnewdt = std::numeric_limits<Real>::max();
  args.jprops = nullptr;
  args.njprop = 0;
  args.period = 0;
  args.call(_uel);

  AbaqusUserElement::updatePNewDt(_min_pnewdt, args.pnewdt);

  if (_uel_uo._nstatev)
    libmesh_map_find(_uel_uo._statev[_uel_uo._statev_index_current], elem->id()) = _statev_copy;
  if (_uel_uo._use_energy)
    libmesh_map_find(_uel_uo._energy, elem->id()) = _energy_copy;

  _uel_uo.assembleUEL(_fe_problem.assembly(_tid, _sys.number()),
                      _local_re,
                      _local_ke,
                      _local_ke_T,
                      _all_dof_indices,
                      do_residual,
                      do_jacobian);
}

void
UELThread::pre()
{
  // reset pnewdt for the next element loop
  _min_pnewdt = std::numeric_limits<Real>::max();
}

void
UELThread::join(const UELThread & other)
{
  AbaqusUserElement::updatePNewDt(_min_pnewdt, other._min_pnewdt);
}
