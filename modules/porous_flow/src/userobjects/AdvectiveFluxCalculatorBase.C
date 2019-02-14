//* This file is part of the MOOSE framework
//* https://www.mooseframework.org All rights reserved, see COPYRIGHT
//* for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT Licensed
//* under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectiveFluxCalculatorBase.h"

#include "libmesh/string_to_enum.h"
#include "libmesh/mesh_tools.h"

template <>
InputParameters
validParams<AdvectiveFluxCalculatorBase>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription(
      "Base class to compute K_ij (a measure of advective flux from node i to node j) "
      "and R+ and R- (which quantify amount of antidiffusion to add) in the "
      "Kuzmin-Turek FEM-TVD multidimensional scheme");
  MooseEnum flux_limiter_type("MinMod VanLeer MC superbee None", "VanLeer");
  params.addParam<MooseEnum>("flux_limiter_type",
                             flux_limiter_type,
                             "Type of flux limiter to use.  'None' means that no antidiffusion "
                             "will be added in the Kuzmin-Turek scheme");
  params.registerRelationshipManagers("ElementSideNeighborLayers");
  params.addPrivateParam<unsigned short>("element_side_neighbor_layers", 2);
  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_LINEAR};
  return params;
}

AdvectiveFluxCalculatorBase::AdvectiveFluxCalculatorBase(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _resizing_needed(true),
    _flux_limiter_type(getParam<MooseEnum>("flux_limiter_type").getEnum<FluxLimiterTypeEnum>()),
    _kij({}),
    _flux_out({}),
    _dflux_out_du({}),
    _dflux_out_dKjk({}),
    _valence({}),
    _u_nodal(),
    _u_nodal_computed_by_thread(),
    _connections()
{
  if (!_execute_enum.contains(EXEC_LINEAR))
    paramError(
        "execute_on",
        "The AdvectiveFluxCalculator UserObject " + name() +
            " execute_on parameter must include, at least, 'linear'.  This is to ensure that "
            "this UserObject computes all necessary quantities just before the Kernels evaluate "
            "their Residuals");
}

void
AdvectiveFluxCalculatorBase::timestepSetup()
{
  // If needed, size and initialize quantities appropriately, and compute _valence
  if (_resizing_needed)
  {
    _connections.clear();
    // QUERY: does this properly account for multiple processors, threading, and other things i
    // haven't thought about?

    /*
     * Populate _connections for all nodes that can be seen by this processor and on relevant blocks
     *
     * MULTIPROC NOTE: this must loop over local elements and 2 layers of ghosted elements.
     * The Kernel will only loop over local elements, so will only use _kij, etc, for
     * linked node-node pairs that appear in the local elements.  Nevertheless, we
     * need to build _kij, etc, for the nodes in the ghosted elements in order to simplify
     * Jacobian computations
     */
    for (const auto & elem : _subproblem.mesh().getMesh().active_element_ptr_range())
      if (this->hasBlocks(elem->subdomain_id()))
        for (unsigned i = 0; i < elem->n_nodes(); ++i)
          _connections.addGlobalNode(elem->node_id(i));
    _connections.finalizeAddingGlobalNodes();
    for (const auto & elem : _subproblem.mesh().getMesh().active_element_ptr_range())
      if (this->hasBlocks(elem->subdomain_id()))
        for (unsigned i = 0; i < elem->n_nodes(); ++i)
          for (unsigned j = 0; j < elem->n_nodes(); ++j)
	    _connections.addConnection(elem->node_id(i), elem->node_id(j));
    _connections.finalizeAddingConnections();
	      

    // initialize _kij
    _kij.clear();
    for (const auto & node_i : _connections.globalIDs())
      if (_kij.find(node_i) == _kij.end())
      {
	_kij[node_i] = {};
	for (const auto & node_j : _connections.globalConnectionsToGlobalID(node_i))
	  _kij[node_i][node_j] = 0.0;
      }


    /*
     * Build _valence[i_j], which is the number of times the i_j pair is encountered when looping
     * over the entire mesh (and on relevant blocks)
     *
     * MULTIPROC NOTE: this must loop over local elements and >=1 layer of ghosted elements.
     * The Kernel will only loop over local elements, so will only use _valence for
     * linked node-node pairs that appear in the local elements.  But other processors will
     * loop over neighboring elements, so avoid multiple counting of the residual and Jacobian
     * this processor must record how many times each node-node link of its local elements
     * appears in the *global* mesh and pass that info to the Kernel
     */
    _valence.clear();
    for (const auto & elem : _fe_problem.getEvaluableElementRange())
    {
      if (this->hasBlocks(elem->subdomain_id()))
      {
        for (unsigned i = 0; i < elem->n_nodes(); ++i)
        {
          const dof_id_type node_i = elem->node_id(i);
          for (unsigned j = 0; j < elem->n_nodes(); ++j)
          {
            const dof_id_type node_j = elem->node_id(j);
            const std::pair<dof_id_type, dof_id_type> i_j(node_i, node_j);
            if (_valence.find(i_j) == _valence.end())
              _valence[i_j] = 0;
            _valence[i_j] += 1;
          }
        }
      }
    }

    _u_nodal.clear();
    _u_nodal_computed_by_thread.clear();
    _flux_out.clear();
    _dflux_out_du.clear();
    _dflux_out_dKjk.clear();
    for (const auto & node_i : _connections.globalIDs())
    {
      _u_nodal.set(node_i, 0.0);
      _u_nodal_computed_by_thread.set(node_i, false);
      _flux_out[node_i] = 0.0;
      _dflux_out_du[node_i] = {};
      zeroedConnection(_dflux_out_du[node_i], node_i);
      _dflux_out_dKjk[node_i] = {};
      for (const auto & node_j : _connections.globalConnectionsToGlobalID(node_i))
      {
        _dflux_out_dKjk[node_i][node_j] = {};
        zeroedConnection(_dflux_out_dKjk[node_i][node_j], node_j);
      }
    }
    _resizing_needed = false;
  }
}

void
AdvectiveFluxCalculatorBase::meshChanged()
{
  ElementUserObject::meshChanged();

  // Signal that _kij, _valence, etc need to be rebuilt
  _resizing_needed = true;
}

void
AdvectiveFluxCalculatorBase::initialize()
{
  // Zero _kij and falsify _u_nodal_computed_by_thread ready for building in execute() and
  // finalize()
  for (const auto & node_i : _connections.globalIDs())
  {
    _u_nodal_computed_by_thread.set(node_i, false);
    for (const auto & node_j : _connections.globalConnectionsToGlobalID(node_i))
      _kij[node_i][node_j] = 0.0;
  }
}

void
AdvectiveFluxCalculatorBase::execute()
{
  // compute _kij contributions from this element that is local to this processor
  // and record _u_nodal
  for (unsigned i = 0; i < _current_elem->n_nodes(); ++i)
  {
    const dof_id_type node_i = _current_elem->node_id(i);
    if (!_u_nodal_computed_by_thread(node_i))
    {
      _u_nodal.set(node_i, computeU(i));
      _u_nodal_computed_by_thread.set(node_i, true);
    }
    for (unsigned j = 0; j < _current_elem->n_nodes(); ++j)
    {
      const dof_id_type node_j = _current_elem->node_id(j);
      for (unsigned qp = 0; qp < _qrule->n_points(); ++qp)
        executeOnElement(node_i, node_j, i, j, qp);
    }
  }
}

void
AdvectiveFluxCalculatorBase::executeOnElement(
    dof_id_type global_i, dof_id_type global_j, unsigned local_i, unsigned local_j, unsigned qp)
{
  // KT Eqn (20)
  _kij[global_i][global_j] += _JxW[qp] * _coord[qp] * computeVelocity(local_i, local_j, qp);
}

void
AdvectiveFluxCalculatorBase::threadJoin(const UserObject & uo)
{
  const AdvectiveFluxCalculatorBase & afc = static_cast<const AdvectiveFluxCalculatorBase &>(uo);
  // add the values of _kij computed by different threads
  /* Can do the following when kij is a std::vector, then do a similar thing for _u_nodal
  for (const auto & node_i : _connections.globalIDs())
    for (const auto & node_j : _connections.globalConnectionsToGlobalID(node_i))
      _kij[node_i][node_j] += afc._kij[node_i][node_j];
  */
  for (const auto & nodes : afc._kij)
  {
    const dof_id_type i = nodes.first;
    for (const auto & neighbors : nodes.second)
    {
      const dof_id_type j = neighbors.first;
      const Real afc_val = neighbors.second;
      _kij[i][j] += afc_val;
    }
  }

  // gather the values of _u_nodal computed by different threads
  for (const auto & node_u : afc._u_nodal)
  {
    const dof_id_type i = node_u.first;
    if (!_u_nodal_computed_by_thread(i) && afc._u_nodal_computed_by_thread(i))
      _u_nodal.set(i, node_u.second);
  }
}

void
AdvectiveFluxCalculatorBase::finalize()
{
  // Overall: ensure _kij is fully built, then compute Kuzmin-Turek D, L, f^a and
  // relevant Jacobian information, and then the relevant quantities into _flux_out and
  // _dflux_out_du, _dflux_out_dKjk

  /*
   * MULTIPROC NOTE: execute() only computed contributions from the elements
   * local to this processor.  Now do the same for the 2 layers of ghosted elements
   */
  for (const auto & elem_id : _fe_problem.ghostedElems())
  {
    _current_elem = _mesh.elem(elem_id);
    if (this->hasBlocks(_current_elem->subdomain_id()))
    {
      _fe_problem.prepare(_current_elem, _tid);
      _fe_problem.reinitElem(_current_elem, _tid);
      execute();
    }
  }

  // Calculate KuzminTurek D matrix
  // See Eqn (32)
  // This adds artificial diffusion, which eliminates any spurious oscillations
  // The idea is that D will remove all negative off-diagonal elements when it is added to K
  // This is identical to full upwinding
  std::map<dof_id_type, std::map<dof_id_type, Real>> dij;
  std::map<dof_id_type, std::map<dof_id_type, Real>>
      dDij_dKij; // dDij_dKij[i][j] = d(D[i][j])/d(K[i][j]) for i!=j
  std::map<dof_id_type, std::map<dof_id_type, Real>>
      dDij_dKji; // dDij_dKji[i][j] = d(D[i][j])/d(K[j][i]) for i!=j
  std::map<dof_id_type, std::map<dof_id_type, Real>>
      dDii_dKij; // dDii_dKij[i][j] = d(D[i][i])/d(K[i][j])
  std::map<dof_id_type, std::map<dof_id_type, Real>>
      dDii_dKji; // dDii_dKji[i][j] = d(D[i][i])/d(K[j][i])
  for (const auto & i : _connections.globalIDs())
  {
    dij[i] = {};
    dij[i][i] = 0.0;
    dDij_dKij[i] = {};
    zeroedConnection(dDij_dKij[i], i);
    dDij_dKji[i] = {};
    zeroedConnection(dDij_dKji[i], i);
    dDii_dKij[i] = {};
    zeroedConnection(dDii_dKij[i], i);
    dDii_dKji[i] = {};
    zeroedConnection(dDii_dKji[i], i);
    for (const auto & j : _connections.globalConnectionsToGlobalID(i))
    {
      if (i == j)
        continue;
      if ((_kij[i][j] <= _kij[j][i]) && (_kij[i][j] < 0))
      {
        dij[i][j] = -_kij[i][j];
        dDij_dKij[i][j] = -1.0;
        dDii_dKij[i][j] += 1.0;
      }
      else if ((_kij[j][i] <= _kij[i][j]) && (_kij[j][i] < 0))
      {
        dij[i][j] = -_kij[j][i];
        dDij_dKji[i][j] = -1.0;
        dDii_dKji[i][j] += 1.0;
      }
      else
        dij[i][j] = 0.0;
      dij[i][i] -= dij[i][j];
    }
  }

  // Calculate KuzminTurek L matrix
  // See Fig 2: L = K + D
  std::map<dof_id_type, std::map<dof_id_type, Real>> lij;
  for (const auto & i : _connections.globalIDs())
  {
    lij[i] = {};
    for (const auto j : _connections.globalConnectionsToGlobalID(i))
      lij[i][j] = _kij[i][j] + dij[i][j];
  }

  // Compute KuzminTurek R matrices
  // See Eqns (49) and (12)
  std::map<dof_id_type, Real> rP;
  std::map<dof_id_type, Real> rM;
  std::map<dof_id_type, std::map<dof_id_type, Real>>
      drP; // drP[i][j] = d(rP[i])/d(u[j]).  Here j must be connected to i (ie _kij[i][j] exists)
           // for there to be a nonzero derivative
  std::map<dof_id_type, std::map<dof_id_type, Real>> drM;
  std::map<dof_id_type, std::map<dof_id_type, Real>>
      drP_dk; // drP_dk[i][j] = d(rP[i])/d(K[i][j]).   Here j must be connected to i (ie _kij[i][j]
              // exists) for there to be a nonzero derivative
  std::map<dof_id_type, std::map<dof_id_type, Real>> drM_dk;
  for (const auto & i : _connections.globalIDs())
  {
    drP[i] = {};
    drP_dk[i] = {};
    rP[i] = rPlus(i, drP[i], drP_dk[i]);
    drM[i] = {};
    drM_dk[i] = {};
    rM[i] = rMinus(i, drM[i], drM_dk[i]);
  }

  // Calculate KuzminTurek f^{a} matrix
  // This is the antidiffusive flux
  // See Eqn (50)
  std::map<dof_id_type, std::map<dof_id_type, Real>> fa;
  std::map<dof_id_type, std::map<dof_id_type, std::map<dof_id_type, Real>>>
      dfa; // dfa[i][j][k] = d(fa[i][j])/du[k].  Here k can be a neighbour to i or a neighbour to j
  std::map<dof_id_type, std::map<dof_id_type, std::map<dof_id_type, Real>>>
      dFij_dKik; // dFij_dKik[i][j][k] = d(fa[i][j])/d(K[i][k]).  Here j must be connected to i, and
                 // k connected to i
  std::map<dof_id_type, std::map<dof_id_type, std::map<dof_id_type, Real>>>
      dFij_dKjk; // dFij_dKjk[i][j][k] = d(fa[i][j])/d(K[j][k]).  Here j must be connected to i, and
                 // k connected to j (including i itself)
  for (const auto & i : _connections.globalIDs())
  {
    fa[i] = {};
    dfa[i] = {};
    dFij_dKik[i] = {};
    dFij_dKjk[i] = {};
    for (const auto & j : _connections.globalConnectionsToGlobalID(i))
    {
      fa[i][j] = 0.0;
      dfa[i][j] = {};
      dFij_dKik[i][j] = {};
      dFij_dKjk[i][j] = {};
      // The derivatives are a bit complicated.
      // If i is upwind of j then fa[i][j] depends on all nodes connected to i.
      // But if i is downwind of j then fa[i][j] depends on all nodes connected to j.
      for (const auto & neighbor_to_i : _connections.globalConnectionsToGlobalID(i))
      {
        dfa[i][j][neighbor_to_i] = 0.0;
        dFij_dKik[i][j][neighbor_to_i] = 0.0;
      }
      for (const auto & neighbor_to_j : _connections.globalConnectionsToGlobalID(j))
      {
        dfa[i][j][neighbor_to_j] = 0.0;
        dFij_dKjk[i][j][neighbor_to_j] = 0.0;
      }
    }
  }

  for (const auto & i : _connections.globalIDs())
  {
    const Real u_i = getUnodal(i);
    for (const auto & j : _connections.globalConnectionsToGlobalID(i))
    {
      const Real u_j = getUnodal(j);
      if (i == j)
        continue;
      if (lij[j][i] >= lij[i][j]) // node i is upwind of node j.
      {
        Real prefactor = 0.0;
        std::map<dof_id_type, Real>
            dprefactor_du; // dprefactor_du[global_id] = d(prefactor)/du[global_id];
        for (const auto & dof_deriv : drP[i])
          dprefactor_du[dof_deriv.first] = 0.0;
        std::map<dof_id_type, Real>
            dprefactor_dKij; // dprefactor_dKij[global_id] = d(prefactor)/dKij[i][global_id].  Here
                             // global_id must be connected to i for a nonzero derivative
        zeroedConnection(dprefactor_dKij, i);
        Real dprefactor_dKji = 0; // dprefactor_dKji = d(prefactor)/dKij[j][i]
        if (u_i >= u_j)
        {
          if (lij[j][i] <= rP[i] * dij[i][j])
          {
            prefactor = lij[j][i];
            dprefactor_dKij[j] += dDij_dKji[j][i];
            dprefactor_dKji += 1.0 + dDij_dKij[j][i];
          }
          else
          {
            prefactor = rP[i] * dij[i][j];
            for (const auto & dof_deriv : drP[i])
              dprefactor_du[dof_deriv.first] = dof_deriv.second * dij[i][j];
            dprefactor_dKij[j] += rP[i] * dDij_dKij[i][j];
            dprefactor_dKji += rP[i] * dDij_dKji[i][j];
            for (const auto & dof_deriv : drP_dk[i])
              dprefactor_dKij[dof_deriv.first] += dof_deriv.second * dij[i][j];
          }
        }
        else
        {
          if (lij[j][i] <= rM[i] * dij[i][j])
          {
            prefactor = lij[j][i];
            dprefactor_dKij[j] += dDij_dKji[j][i];
            dprefactor_dKji += 1.0 + dDij_dKij[j][i];
          }
          else
          {
            prefactor = rM[i] * dij[i][j];
            for (const auto & dof_deriv : drM[i])
              dprefactor_du[dof_deriv.first] = dof_deriv.second * dij[i][j];
            dprefactor_dKij[j] += rM[i] * dDij_dKij[i][j];
            dprefactor_dKji += rM[i] * dDij_dKji[i][j];
            for (const auto & dof_deriv : drM_dk[i])
              dprefactor_dKij[dof_deriv.first] += dof_deriv.second * dij[i][j];
          }
        }
        fa[i][j] = prefactor * (u_i - u_j);
        dfa[i][j][i] = prefactor;
        dfa[i][j][j] = -prefactor;
        for (const auto & dof_deriv : dprefactor_du)
          dfa[i][j][dof_deriv.first] += dof_deriv.second * (u_i - u_j);
        for (const auto & dof_deriv : dprefactor_dKij)
          dFij_dKik[i][j][dof_deriv.first] += dof_deriv.second * (u_i - u_j);
        dFij_dKjk[i][j][i] += dprefactor_dKji * (u_i - u_j);
      }
    }
  }
  for (const auto & i : _connections.globalIDs())
  {
    for (const auto & j : _connections.globalConnectionsToGlobalID(i))
    {
      if (i == j)
        continue;
      if (lij[j][i] < lij[i][j]) // node_i is downwind of node_j.
      {
        fa[i][j] = -fa[j][i];
        for (const auto & dof_deriv : dfa[j][i])
          dfa[i][j][dof_deriv.first] = -dof_deriv.second;
        for (const auto & dof_deriv : dFij_dKjk[j][i])
          dFij_dKik[i][j][dof_deriv.first] = -dof_deriv.second;
        for (const auto & dof_deriv : dFij_dKik[j][i])
          dFij_dKjk[i][j][dof_deriv.first] = -dof_deriv.second;
      }
    }
  }

  // zero _flux_out and its derivatives
  for (const auto & i : _connections.globalIDs())
  {
    _flux_out[i] = 0.0;
    // The derivatives are a bit complicated.
    // If i is upwind of a node "j" then _flux_out[i] depends on all nodes connected to i.
    // But if i is downwind of a node "j" then _flux_out depends on all nodes connected with node
    // j.
    for (const auto & j : _connections.globalConnectionsToGlobalID(i))
    {
      _dflux_out_du[i][j] = 0.0;
      for (const auto & neighbors_j : _connections.globalConnectionsToGlobalID(j))
      {
        _dflux_out_du[i][neighbors_j] = 0.0;
        _dflux_out_dKjk[i][j][neighbors_j] = 0.0;
      }
    }
  }

  // Add everything together
  // See step 3 in Fig 2, noting Eqn (36)
  for (const auto & i : _connections.globalIDs())
  {
    for (const auto & j : _connections.globalConnectionsToGlobalID(i))
    {
      const Real u_j = getUnodal(j);

      // negative sign because residual = -Lu (KT equation (19))
      _flux_out[i] -= lij[i][j] * u_j + fa[i][j];

      _dflux_out_du[i][j] -= lij[i][j];
      for (const auto & dof_deriv : dfa[i][j])
        _dflux_out_du[i][dof_deriv.first] -= dof_deriv.second;

      _dflux_out_dKjk[i][i][j] -= 1.0 * u_j; // from the K in L = K + D

      if (j == i)
        for (const auto & dof_deriv : dDii_dKij[i])
          _dflux_out_dKjk[i][i][dof_deriv.first] -= dof_deriv.second * u_j;
      else
        _dflux_out_dKjk[i][i][j] -= dDij_dKij[i][j] * u_j;
      for (const auto & dof_deriv : dFij_dKik[i][j])
        _dflux_out_dKjk[i][i][dof_deriv.first] -= dof_deriv.second;

      if (j == i)
        for (const auto & dof_deriv : dDii_dKji[i])
          _dflux_out_dKjk[i][dof_deriv.first][i] -= dof_deriv.second * u_j;
      else
        _dflux_out_dKjk[i][j][i] -= dDij_dKji[i][j] * u_j;
      for (const auto & dof_deriv : dFij_dKjk[i][j])
        _dflux_out_dKjk[i][j][dof_deriv.first] -= dof_deriv.second;
    }
  }
}

Real
AdvectiveFluxCalculatorBase::rPlus(dof_id_type node_i,
                                   std::map<dof_id_type, Real> & dlimited_du,
                                   std::map<dof_id_type, Real> & dlimited_dk) const
{
  zeroedConnection(dlimited_du, node_i);
  zeroedConnection(dlimited_dk, node_i);
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;
  std::map<dof_id_type, Real> dp_du;
  std::map<dof_id_type, Real> dp_dk;
  const Real p = PQPlusMinus(node_i, PQPlusMinusEnum::PPlus, dp_du, dp_dk);
  if (p == 0.0)
    // Comment after Eqn (49): if P=0 then there's no antidiffusion, so no need to remove it
    return 1.0;
  std::map<dof_id_type, Real> dq_du;
  std::map<dof_id_type, Real> dq_dk;
  const Real q = PQPlusMinus(node_i, PQPlusMinusEnum::QPlus, dq_du, dq_dk);
  const Real r = q / p;
  std::map<dof_id_type, Real> dr_du;
  for (const auto & node_deriv : dp_du)
    dr_du[node_deriv.first] = dq_du[node_deriv.first] / p - q * node_deriv.second / std::pow(p, 2);
  std::map<dof_id_type, Real> dr_dk;
  for (const auto & node_deriv : dp_dk)
    dr_dk[node_deriv.first] = dq_dk[node_deriv.first] / p - q * node_deriv.second / std::pow(p, 2);
  Real limited;
  Real dlimited_dr;
  limitFlux(1.0, r, limited, dlimited_dr);
  for (const auto & node_deriv : dr_du)
    dlimited_du[node_deriv.first] = dlimited_dr * node_deriv.second;
  for (const auto & node_deriv : dr_dk)
    dlimited_dk[node_deriv.first] = dlimited_dr * node_deriv.second;
  return limited;
}

Real
AdvectiveFluxCalculatorBase::rMinus(dof_id_type node_i,
                                    std::map<dof_id_type, Real> & dlimited_du,
                                    std::map<dof_id_type, Real> & dlimited_dk) const
{
  zeroedConnection(dlimited_du, node_i);
  zeroedConnection(dlimited_dk, node_i);
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;
  std::map<dof_id_type, Real> dp_du;
  std::map<dof_id_type, Real> dp_dk;
  const Real p = PQPlusMinus(node_i, PQPlusMinusEnum::PMinus, dp_du, dp_dk);
  if (p == 0.0)
    // Comment after Eqn (49): if P=0 then there's no antidiffusion, so no need to remove it
    return 1.0;
  std::map<dof_id_type, Real> dq_du;
  std::map<dof_id_type, Real> dq_dk;
  const Real q = PQPlusMinus(node_i, PQPlusMinusEnum::QMinus, dq_du, dq_dk);
  const Real r = q / p;
  std::map<dof_id_type, Real> dr_du;
  for (const auto & node_deriv : dp_du)
    dr_du[node_deriv.first] = dq_du[node_deriv.first] / p - q * node_deriv.second / std::pow(p, 2);
  std::map<dof_id_type, Real> dr_dk;
  for (const auto & node_deriv : dp_dk)
    dr_dk[node_deriv.first] = dq_dk[node_deriv.first] / p - q * node_deriv.second / std::pow(p, 2);
  Real limited;
  Real dlimited_dr;
  limitFlux(1.0, r, limited, dlimited_dr);
  for (const auto & node_deriv : dr_du)
    dlimited_du[node_deriv.first] = dlimited_dr * node_deriv.second;
  for (const auto & node_deriv : dr_dk)
    dlimited_dk[node_deriv.first] = dlimited_dr * node_deriv.second;
  return limited;
}

void
AdvectiveFluxCalculatorBase::limitFlux(Real a, Real b, Real & limited, Real & dlimited_db) const
{
  limited = 0.0;
  dlimited_db = 0.0;
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return;

  if ((a >= 0.0 && b <= 0.0) || (a <= 0.0 && b >= 0.0))
    return;
  const Real s = (a > 0.0 ? 1.0 : -1.0);

  const Real lal = std::abs(a);
  const Real lbl = std::abs(b);
  const Real dlbl = (b >= 0.0 ? 1.0 : -1.0); // d(lbl)/db
  switch (_flux_limiter_type)
  {
    case FluxLimiterTypeEnum::MinMod:
    {
      if (lal <= lbl)
      {
        limited = s * lal;
        dlimited_db = 0.0;
      }
      else
      {
        limited = s * lbl;
        dlimited_db = s * dlbl;
      }
      return;
    }
    case FluxLimiterTypeEnum::VanLeer:
    {
      limited = s * 2 * lal * lbl / (lal + lbl);
      dlimited_db = s * 2 * lal * (dlbl / (lal + lbl) - lbl * dlbl / std::pow(lal + lbl, 2));
      return;
    }
    case FluxLimiterTypeEnum::MC:
    {
      const Real av = 0.5 * std::abs(a + b);
      if (2 * lal <= av && lal <= lbl)
      {
        // 2 * lal is the smallest
        limited = s * 2.0 * lal;
        dlimited_db = 0.0;
      }
      else if (2 * lbl <= av && lbl <= lal)
      {
        // 2 * lbl is the smallest
        limited = s * 2.0 * lbl;
        dlimited_db = s * 2.0 * dlbl;
      }
      else
      {
        // av is the smallest
        limited = s * av;
        // if (a>0 and b>0) then d(av)/db = 0.5 = 0.5 * dlbl
        // if (a<0 and b<0) then d(av)/db = -0.5 = 0.5 * dlbl
        // if a and b have different sign then limited=0, above
        dlimited_db = s * 0.5 * dlbl;
      }
      return;
    }
    case FluxLimiterTypeEnum::superbee:
    {
      const Real term1 = std::min(2.0 * lal, lbl);
      const Real term2 = std::min(lal, 2.0 * lbl);
      if (term1 >= term2)
      {
        if (2.0 * lal <= lbl)
        {
          limited = s * 2 * lal;
          dlimited_db = 0.0;
        }
        else
        {
          limited = s * lbl;
          dlimited_db = s * dlbl;
        }
      }
      else
      {
        if (lal <= 2.0 * lbl)
        {
          limited = s * lal;
          dlimited_db = 0.0;
        }
        else
        {
          limited = s * 2.0 * lbl;
          dlimited_db = s * 2.0 * dlbl;
        }
      }
      return;
    }
    default:
      return;
  }
}

Real
AdvectiveFluxCalculatorBase::getKij(dof_id_type node_i, dof_id_type node_j) const
{

  const auto & row_find = _kij.find(node_i);
  if (row_find == _kij.end())
    mooseError("AdvectiveFluxCalculatorBase UserObject " + name() + " Kij does not contain node " +
               Moose::stringify(node_i));
  const std::map<dof_id_type, Real> & kij_row = row_find->second;
  const auto & entry_find = kij_row.find(node_j);
  if (entry_find == kij_row.end())
    mooseError("AdvectiveFluxCalculatorBase UserObject " + name() + " Kij on row " +
               Moose::stringify(node_i) + " does not contain node " + Moose::stringify(node_j));

  return entry_find->second;
}

const std::map<dof_id_type, Real> &
AdvectiveFluxCalculatorBase::getdFluxOutdu(dof_id_type node_i) const
{
  const auto & row_find = _dflux_out_du.find(node_i);
  if (row_find == _dflux_out_du.end())
    mooseError("AdvectiveFluxCalculatorBase UserObject " + name() +
               " _dflux_out_du does not contain node " + Moose::stringify(node_i));
  return row_find->second;
}

const std::map<dof_id_type, std::map<dof_id_type, Real>> &
AdvectiveFluxCalculatorBase::getdFluxOutdKjk(dof_id_type node_i) const
{
  const auto & row_find = _dflux_out_dKjk.find(node_i);
  if (row_find == _dflux_out_dKjk.end())
    mooseError("AdvectiveFluxCalculatorBase UserObject " + name() +
               " _dflux_out_dKjk does not contain node " + Moose::stringify(node_i));
  return row_find->second;
}

Real
AdvectiveFluxCalculatorBase::getFluxOut(dof_id_type node_i) const
{
  const auto & entry_find = _flux_out.find(node_i);
  if (entry_find == _flux_out.end())
    mooseError("AdvectiveFluxCalculatorBase UserObject " + name() +
               " _flux_out does not contain node " + Moose::stringify(node_i));
  return entry_find->second;
}

unsigned
AdvectiveFluxCalculatorBase::getValence(dof_id_type node_i, dof_id_type node_j) const
{
  const std::pair<dof_id_type, dof_id_type> i_j(node_i, node_j);
  const auto & entry_find = _valence.find(i_j);
  if (entry_find == _valence.end())
    mooseError("AdvectiveFluxCalculatorBase UserObject " + name() +
               " Valence does not contain node-pair " + Moose::stringify(node_i) + " to " +
               Moose::stringify(node_j));
  return entry_find->second;
}

void
AdvectiveFluxCalculatorBase::zeroedConnection(std::map<dof_id_type, Real> & the_map,
                                              dof_id_type node_i) const
{
  the_map.clear();
  for (const auto & node_j : _connections.globalConnectionsToGlobalID(node_i))
    the_map[node_j] = 0.0;
}

Real
AdvectiveFluxCalculatorBase::PQPlusMinus(dof_id_type node_i,
                                         const PQPlusMinusEnum pq_plus_minus,
                                         std::map<dof_id_type, Real> & derivs,
                                         std::map<dof_id_type, Real> & dpqdk) const
{
  // Find the value of u at node_i
  const Real u_i = getUnodal(node_i);

  // We're going to sum over all nodes connected with node_i
  // These nodes are found in _kij[node_i]
  const auto & row_find = _kij.find(node_i);
  if (row_find == _kij.end())
    mooseError("AdvectiveFluxCalculatorBase UserObject " + name() + " Kij does not contain node " +
               Moose::stringify(node_i));
  const std::map<dof_id_type, Real> nodej_and_kij = row_find->second;

  // Initialize the results
  Real result = 0.0;
  zeroedConnection(derivs, node_i);
  zeroedConnection(dpqdk, node_i);

  // Sum over all nodes connected with node_i.
  for (const auto & nk : nodej_and_kij)
  {
    const dof_id_type node_j = nk.first;
    const Real kentry = nk.second;

    // Find the value of u at node_j
    const Real u_j = getUnodal(node_j);
    const Real ujminusi = u_j - u_i;

    // Evaluate the i-j contribution to the result
    switch (pq_plus_minus)
    {
      case PQPlusMinusEnum::PPlus:
      {
        if (ujminusi < 0.0 && kentry < 0.0)
        {
          result += kentry * ujminusi;
          derivs[node_j] += kentry;
          derivs[node_i] -= kentry;
          dpqdk[node_j] += ujminusi;
        }
        break;
      }
      case PQPlusMinusEnum::PMinus:
      {
        if (ujminusi > 0.0 && kentry < 0.0)
        {
          result += kentry * ujminusi;
          derivs[node_j] += kentry;
          derivs[node_i] -= kentry;
          dpqdk[node_j] += ujminusi;
        }
        break;
      }
      case PQPlusMinusEnum::QPlus:
      {
        if (ujminusi > 0.0 && kentry > 0.0)
        {
          result += kentry * ujminusi;
          derivs[node_j] += kentry;
          derivs[node_i] -= kentry;
          dpqdk[node_j] += ujminusi;
        }
        break;
      }
      case PQPlusMinusEnum::QMinus:
      {
        if (ujminusi < 0.0 && kentry > 0.0)
        {
          result += kentry * ujminusi;
          derivs[node_j] += kentry;
          derivs[node_i] -= kentry;
          dpqdk[node_j] += ujminusi;
        }
        break;
      }
    }
  }

  return result;
}

Real
AdvectiveFluxCalculatorBase::getUnodal(dof_id_type node_i) const
{
  return _u_nodal(node_i);
}
