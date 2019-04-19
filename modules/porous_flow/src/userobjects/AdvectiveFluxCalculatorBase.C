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

  params.addRelationshipManager("ElementPointNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC,
                                [](const InputParameters &, InputParameters & rm_params) {
                                  rm_params.set<unsigned short>("layers") = 2;
                                });

  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_LINEAR};
  return params;
}

AdvectiveFluxCalculatorBase::AdvectiveFluxCalculatorBase(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _resizing_needed(true),
    _flux_limiter_type(getParam<MooseEnum>("flux_limiter_type").getEnum<FluxLimiterTypeEnum>()),
    _kij(),
    _flux_out(),
    _dflux_out_du(),
    _dflux_out_dKjk(),
    _valence(),
    _u_nodal(),
    _u_nodal_computed_by_thread(),
    _connections(),
    _number_of_nodes(0),
    _my_pid(processor_id()),
    _nodes_to_receive(_app.n_processors()),
    _nodes_to_send(_app.n_processors()),
    _pairs_to_receive(_app.n_processors()),
    _pairs_to_send(_app.n_processors())
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
    /*
     * Populate _connections for all nodes that can be seen by this processor and on relevant
     * blocks
     *
     * MULTIPROC NOTE: this must loop over local elements and 2 layers of ghosted elements.
     * The Kernel will only loop over local elements, so will only use _kij, etc, for
     * linked node-node pairs that appear in the local elements.  Nevertheless, we
     * need to build _kij, etc, for the nodes in the ghosted elements in order to simplify
     * Jacobian computations
     */
    _connections.clear();
    for (const auto & elem : _fe_problem.getEvaluableElementRange())
      if (this->hasBlocks(elem->subdomain_id()))
        for (unsigned i = 0; i < elem->n_nodes(); ++i)
          _connections.addGlobalNode(elem->node_id(i));
    _connections.finalizeAddingGlobalNodes();
    for (const auto & elem : _fe_problem.getEvaluableElementRange())
      if (this->hasBlocks(elem->subdomain_id()))
        for (unsigned i = 0; i < elem->n_nodes(); ++i)
          for (unsigned j = 0; j < elem->n_nodes(); ++j)
            _connections.addConnection(elem->node_id(i), elem->node_id(j));
    _connections.finalizeAddingConnections();

    _number_of_nodes = _connections.numNodes();

    // initialize _kij
    _kij.resize(_number_of_nodes);
    for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
      _kij[sequential_i].assign(
          _connections.sequentialConnectionsToSequentialID(sequential_i).size(), 0.0);

    /*
     * Build _valence[i], which is the number of times the sequential node i is encountered when
     * looping over the entire mesh (and on relevant blocks)
     *
     * MULTIPROC NOTE: this must loop over local elements and >=1 layer of ghosted elements.
     * The Kernel will only loop over local elements, so will only use _valence for
     * linked node-node pairs that appear in the local elements.  But other processors will
     * loop over neighboring elements, so avoid multiple counting of the residual and Jacobian
     * this processor must record how many times each node-node link of its local elements
     * appears in the local elements and >=1 layer, and pass that info to the Kernel
     */
    _valence.assign(_number_of_nodes, 0);
    for (const auto & elem : _fe_problem.getEvaluableElementRange())
      if (this->hasBlocks(elem->subdomain_id()))
        for (unsigned i = 0; i < elem->n_nodes(); ++i)
        {
          const dof_id_type node_i = elem->node_id(i);
          const dof_id_type sequential_i = _connections.sequentialID(node_i);
          _valence[sequential_i] += 1;
        }

    _u_nodal.assign(_number_of_nodes, 0.0);
    _u_nodal_computed_by_thread.assign(_number_of_nodes, false);
    _flux_out.assign(_number_of_nodes, 0.0);
    _dflux_out_du.assign(_number_of_nodes, std::map<dof_id_type, Real>());
    for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
      zeroedConnection(_dflux_out_du[sequential_i], _connections.globalID(sequential_i));
    _dflux_out_dKjk.resize(_number_of_nodes);
    for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
    {
      const std::vector<dof_id_type> con_i =
          _connections.sequentialConnectionsToSequentialID(sequential_i);
      const std::size_t num_con_i = con_i.size();
      _dflux_out_dKjk[sequential_i].resize(num_con_i);
      for (std::size_t j = 0; j < con_i.size(); ++j)
      {
        const dof_id_type sequential_j = con_i[j];
        const std::size_t num_con_j =
            _connections.sequentialConnectionsToSequentialID(sequential_j).size();
        _dflux_out_dKjk[sequential_i][j].assign(num_con_j, 0.0);
      }
    }

    if (_app.n_processors() > 1)
      buildCommLists();

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
  _u_nodal_computed_by_thread.assign(_number_of_nodes, false);
  for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
    _kij[sequential_i].assign(_connections.sequentialConnectionsToSequentialID(sequential_i).size(),
                              0.0);
}

void
AdvectiveFluxCalculatorBase::execute()
{
  // compute _kij contributions from this element that is local to this processor
  // and record _u_nodal
  for (unsigned i = 0; i < _current_elem->n_nodes(); ++i)
  {
    const dof_id_type node_i = _current_elem->node_id(i);
    const dof_id_type sequential_i = _connections.sequentialID(node_i);
    if (!_u_nodal_computed_by_thread[sequential_i])
    {
      _u_nodal[sequential_i] = computeU(i);
      _u_nodal_computed_by_thread[sequential_i] = true;
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
  const dof_id_type sequential_i = _connections.sequentialID(global_i);
  const unsigned index_i_to_j = _connections.indexOfGlobalConnection(global_i, global_j);
  _kij[sequential_i][index_i_to_j] += _JxW[qp] * _coord[qp] * computeVelocity(local_i, local_j, qp);
}

void
AdvectiveFluxCalculatorBase::threadJoin(const UserObject & uo)
{
  const AdvectiveFluxCalculatorBase & afc = static_cast<const AdvectiveFluxCalculatorBase &>(uo);
  // add the values of _kij computed by different threads
  for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
  {
    const std::size_t num_con_i =
        _connections.sequentialConnectionsToSequentialID(sequential_i).size();
    for (std::size_t j = 0; j < num_con_i; ++j)
      _kij[sequential_i][j] += afc._kij[sequential_i][j];
  }

  // gather the values of _u_nodal computed by different threads
  for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
    if (!_u_nodal_computed_by_thread[sequential_i] && afc._u_nodal_computed_by_thread[sequential_i])
      _u_nodal[sequential_i] = afc._u_nodal[sequential_i];
}

void
AdvectiveFluxCalculatorBase::finalize()
{
  // Overall: ensure _kij is fully built, then compute Kuzmin-Turek D, L, f^a and
  // relevant Jacobian information, and then the relevant quantities into _flux_out and
  // _dflux_out_du, _dflux_out_dKjk

  if (_app.n_processors() > 1)
    exchangeGhostedInfo();

  // Calculate KuzminTurek D matrix
  // See Eqn (32)
  // This adds artificial diffusion, which eliminates any spurious oscillations
  // The idea is that D will remove all negative off-diagonal elements when it is added to K
  // This is identical to full upwinding
  std::vector<std::vector<Real>> dij(_number_of_nodes);
  std::vector<std::vector<Real>> dDij_dKij(
      _number_of_nodes); // dDij_dKij[i][j] = d(D[i][j])/d(K[i][j]) for i!=j
  std::vector<std::vector<Real>> dDij_dKji(
      _number_of_nodes); // dDij_dKji[i][j] = d(D[i][j])/d(K[j][i]) for i!=j
  std::vector<std::vector<Real>> dDii_dKij(
      _number_of_nodes); // dDii_dKij[i][j] = d(D[i][i])/d(K[i][j])
  std::vector<std::vector<Real>> dDii_dKji(
      _number_of_nodes); // dDii_dKji[i][j] = d(D[i][i])/d(K[j][i])
  for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
  {
    const std::vector<dof_id_type> con_i =
        _connections.sequentialConnectionsToSequentialID(sequential_i);
    const std::size_t num_con_i = con_i.size();
    dij[sequential_i].assign(num_con_i, 0.0);
    dDij_dKij[sequential_i].assign(num_con_i, 0.0);
    dDij_dKji[sequential_i].assign(num_con_i, 0.0);
    dDii_dKij[sequential_i].assign(num_con_i, 0.0);
    dDii_dKji[sequential_i].assign(num_con_i, 0.0);
    const unsigned index_i_to_i =
        _connections.indexOfSequentialConnection(sequential_i, sequential_i);
    for (std::size_t j = 0; j < num_con_i; ++j)
    {
      const dof_id_type sequential_j = con_i[j];
      if (sequential_i == sequential_j)
        continue;
      const unsigned index_j_to_i =
          _connections.indexOfSequentialConnection(sequential_j, sequential_i);
      const Real kij = _kij[sequential_i][j];
      const Real kji = _kij[sequential_j][index_j_to_i];
      if ((kij <= kji) && (kij < 0.0))
      {
        dij[sequential_i][j] = -kij;
        dDij_dKij[sequential_i][j] = -1.0;
        dDii_dKij[sequential_i][j] += 1.0;
      }
      else if ((kji <= kij) && (kji < 0.0))
      {
        dij[sequential_i][j] = -kji;
        dDij_dKji[sequential_i][j] = -1.0;
        dDii_dKji[sequential_i][j] += 1.0;
      }
      else
        dij[sequential_i][j] = 0.0;
      dij[sequential_i][index_i_to_i] -= dij[sequential_i][j];
    }
  }

  // Calculate KuzminTurek L matrix
  // See Fig 2: L = K + D
  std::vector<std::vector<Real>> lij(_number_of_nodes);
  for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
  {
    const std::vector<dof_id_type> con_i =
        _connections.sequentialConnectionsToSequentialID(sequential_i);
    const std::size_t num_con_i = con_i.size();
    lij[sequential_i].assign(num_con_i, 0.0);
    for (std::size_t j = 0; j < num_con_i; ++j)
      lij[sequential_i][j] = _kij[sequential_i][j] + dij[sequential_i][j];
  }

  // Compute KuzminTurek R matrices
  // See Eqns (49) and (12)
  std::vector<Real> rP(_number_of_nodes);
  std::vector<Real> rM(_number_of_nodes);
  std::vector<std::vector<Real>> drP(_number_of_nodes); // drP[i][j] = d(rP[i])/d(u[j]).  Here j
                                                        // indexes the j^th node connected to i
  std::vector<std::vector<Real>> drM(_number_of_nodes);
  std::vector<std::vector<Real>> drP_dk(
      _number_of_nodes); // drP_dk[i][j] = d(rP[i])/d(K[i][j]).  Here j indexes the j^th node
                         // connected to i
  std::vector<std::vector<Real>> drM_dk(_number_of_nodes);
  for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
  {
    rP[sequential_i] = rPlus(sequential_i, drP[sequential_i], drP_dk[sequential_i]);
    rM[sequential_i] = rMinus(sequential_i, drM[sequential_i], drM_dk[sequential_i]);
  }

  // Calculate KuzminTurek f^{a} matrix
  // This is the antidiffusive flux
  // See Eqn (50)
  std::vector<std::vector<Real>> fa(_number_of_nodes); // fa[sequential_i][j]  sequential_j is the
                                                       // j^th connection to sequential_i
  // The derivatives are a bit complicated.
  // If i is upwind of j then fa[i][j] depends on all nodes connected to i.
  // But if i is downwind of j then fa[i][j] depends on all nodes connected to j.
  std::vector<std::vector<std::map<dof_id_type, Real>>> dfa(
      _number_of_nodes); // dfa[sequential_i][j][global_k] = d(fa[sequential_i][j])/du[global_k].
                         // Here global_k can be a neighbor to sequential_i or a neighbour to
                         // sequential_j (sequential_j is the j^th connection to sequential_i)
  std::vector<std::vector<std::vector<Real>>> dFij_dKik(
      _number_of_nodes); // dFij_dKik[sequential_i][j][k] =
                         // d(fa[sequential_i][j])/d(K[sequential_i][k]).  Here j denotes the j^th
                         // connection to sequential_i, while k denotes the k^th connection to
                         // sequential_i
  std::vector<std::vector<std::vector<Real>>> dFij_dKjk(
      _number_of_nodes); // dFij_dKjk[sequential_i][j][k] =
                         // d(fa[sequential_i][j])/d(K[sequential_j][k]).  Here sequential_j is
                         // the j^th connection to sequential_i, and k denotes the k^th connection
                         // to sequential_j (this will include sequential_i itself)
  for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
  {
    const std::vector<dof_id_type> con_i =
        _connections.globalConnectionsToSequentialID(sequential_i);
    const unsigned num_con_i = con_i.size();
    fa[sequential_i].assign(num_con_i, 0.0);
    dfa[sequential_i].resize(num_con_i);
    dFij_dKik[sequential_i].resize(num_con_i);
    dFij_dKjk[sequential_i].resize(num_con_i);
    for (std::size_t j = 0; j < num_con_i; ++j)
    {
      for (const auto & global_k : con_i)
        dfa[sequential_i][j][global_k] = 0;
      const dof_id_type global_j = con_i[j];
      const std::vector<dof_id_type> con_j = _connections.globalConnectionsToGlobalID(global_j);
      const unsigned num_con_j = con_j.size();
      for (const auto & global_k : con_j)
        dfa[sequential_i][j][global_k] = 0;
      dFij_dKik[sequential_i][j].assign(num_con_i, 0.0);
      dFij_dKjk[sequential_i][j].assign(num_con_j, 0.0);
    }
  }

  for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
  {
    const dof_id_type global_i = _connections.globalID(sequential_i);
    const Real u_i = _u_nodal[sequential_i];
    const std::vector<dof_id_type> con_i =
        _connections.sequentialConnectionsToSequentialID(sequential_i);
    const std::size_t num_con_i = con_i.size();
    for (std::size_t j = 0; j < num_con_i; ++j)
    {
      const dof_id_type sequential_j = con_i[j];
      if (sequential_i == sequential_j)
        continue;
      const unsigned index_j_to_i =
          _connections.indexOfSequentialConnection(sequential_j, sequential_i);
      const Real Lij = lij[sequential_i][j];
      const Real Lji = lij[sequential_j][index_j_to_i];
      if (Lji >= Lij) // node i is upwind of node j.
      {
        const Real Dij = dij[sequential_i][j];
        const dof_id_type global_j = _connections.globalID(sequential_j);
        const Real u_j = _u_nodal[sequential_j];
        Real prefactor = 0.0;
        std::vector<Real> dprefactor_du(num_con_i,
                                        0.0); // dprefactor_du[j] = d(prefactor)/du[sequential_j]
        std::vector<Real> dprefactor_dKij(
            num_con_i, 0.0);      // dprefactor_dKij[j] = d(prefactor)/dKij[sequential_i][j].
        Real dprefactor_dKji = 0; // dprefactor_dKji = d(prefactor)/dKij[sequential_j][index_j_to_i]
        if (u_i >= u_j)
        {
          if (Lji <= rP[sequential_i] * Dij)
          {
            prefactor = Lji;
            dprefactor_dKij[j] += dDij_dKji[sequential_j][index_j_to_i];
            dprefactor_dKji += 1.0 + dDij_dKij[sequential_j][index_j_to_i];
          }
          else
          {
            prefactor = rP[sequential_i] * Dij;
            for (std::size_t ind_j = 0; ind_j < num_con_i; ++ind_j)
              dprefactor_du[ind_j] = drP[sequential_i][ind_j] * Dij;
            dprefactor_dKij[j] += rP[sequential_i] * dDij_dKij[sequential_i][j];
            dprefactor_dKji += rP[sequential_i] * dDij_dKji[sequential_i][j];
            for (std::size_t ind_j = 0; ind_j < num_con_i; ++ind_j)
              dprefactor_dKij[ind_j] += drP_dk[sequential_i][ind_j] * Dij;
          }
        }
        else
        {
          if (Lji <= rM[sequential_i] * Dij)
          {
            prefactor = Lji;
            dprefactor_dKij[j] += dDij_dKji[sequential_j][index_j_to_i];
            dprefactor_dKji += 1.0 + dDij_dKij[sequential_j][index_j_to_i];
          }
          else
          {
            prefactor = rM[sequential_i] * Dij;
            for (std::size_t ind_j = 0; ind_j < num_con_i; ++ind_j)
              dprefactor_du[ind_j] = drM[sequential_i][ind_j] * Dij;
            dprefactor_dKij[j] += rM[sequential_i] * dDij_dKij[sequential_i][j];
            dprefactor_dKji += rM[sequential_i] * dDij_dKji[sequential_i][j];
            for (std::size_t ind_j = 0; ind_j < num_con_i; ++ind_j)
              dprefactor_dKij[ind_j] += drM_dk[sequential_i][ind_j] * Dij;
          }
        }
        fa[sequential_i][j] = prefactor * (u_i - u_j);
        dfa[sequential_i][j][global_i] = prefactor;
        dfa[sequential_i][j][global_j] = -prefactor;
        for (std::size_t ind_j = 0; ind_j < num_con_i; ++ind_j)
        {
          dfa[sequential_i][j][_connections.globalID(con_i[ind_j])] +=
              dprefactor_du[ind_j] * (u_i - u_j);
          dFij_dKik[sequential_i][j][ind_j] += dprefactor_dKij[ind_j] * (u_i - u_j);
        }
        dFij_dKjk[sequential_i][j][index_j_to_i] += dprefactor_dKji * (u_i - u_j);
      }
    }
  }

  for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
  {
    const std::vector<dof_id_type> con_i =
        _connections.sequentialConnectionsToSequentialID(sequential_i);
    const std::size_t num_con_i = con_i.size();
    for (std::size_t j = 0; j < num_con_i; ++j)
    {
      const dof_id_type sequential_j = con_i[j];
      if (sequential_i == sequential_j)
        continue;
      const unsigned index_j_to_i =
          _connections.indexOfSequentialConnection(sequential_j, sequential_i);
      if (lij[sequential_j][index_j_to_i] < lij[sequential_i][j]) // node_i is downwind of node_j.
      {
        fa[sequential_i][j] = -fa[sequential_j][index_j_to_i];
        for (const auto & dof_deriv : dfa[sequential_j][index_j_to_i])
          dfa[sequential_i][j][dof_deriv.first] = -dof_deriv.second;
        for (std::size_t k = 0; k < num_con_i; ++k)
          dFij_dKik[sequential_i][j][k] = -dFij_dKjk[sequential_j][index_j_to_i][k];
        const std::size_t num_con_j =
            _connections.sequentialConnectionsToSequentialID(sequential_j).size();
        for (std::size_t k = 0; k < num_con_j; ++k)
          dFij_dKjk[sequential_i][j][k] = -dFij_dKik[sequential_j][index_j_to_i][k];
      }
    }
  }

  // zero _flux_out and its derivatives
  _flux_out.assign(_number_of_nodes, 0.0);
  // The derivatives are a bit complicated.
  // If i is upwind of a node "j" then _flux_out[i] depends on all nodes connected to i.
  // But if i is downwind of a node "j" then _flux_out depends on all nodes connected with node
  // j.
  _dflux_out_du.assign(_number_of_nodes, std::map<dof_id_type, Real>());
  for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
    for (const auto & j : _connections.globalConnectionsToSequentialID(sequential_i))
    {
      _dflux_out_du[sequential_i][j] = 0.0;
      for (const auto & neighbors_j : _connections.globalConnectionsToGlobalID(j))
        _dflux_out_du[sequential_i][neighbors_j] = 0.0;
    }
  _dflux_out_dKjk.resize(_number_of_nodes);
  for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
  {
    const std::vector<dof_id_type> con_i =
        _connections.sequentialConnectionsToSequentialID(sequential_i);
    const std::size_t num_con_i = con_i.size();
    _dflux_out_dKjk[sequential_i].resize(num_con_i);
    for (std::size_t j = 0; j < num_con_i; ++j)
    {
      const dof_id_type sequential_j = con_i[j];
      std::vector<dof_id_type> con_j =
          _connections.sequentialConnectionsToSequentialID(sequential_j);
      _dflux_out_dKjk[sequential_i][j].assign(con_j.size(), 0.0);
    }
  }

  // Add everything together
  // See step 3 in Fig 2, noting Eqn (36)
  for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
  {
    std::vector<dof_id_type> con_i = _connections.sequentialConnectionsToSequentialID(sequential_i);
    const size_t num_con_i = con_i.size();
    const dof_id_type index_i_to_i =
        _connections.indexOfSequentialConnection(sequential_i, sequential_i);
    for (std::size_t j = 0; j < num_con_i; ++j)
    {
      const dof_id_type sequential_j = con_i[j];
      const dof_id_type global_j = _connections.globalID(sequential_j);
      const Real u_j = _u_nodal[sequential_j];

      // negative sign because residual = -Lu (KT equation (19))
      _flux_out[sequential_i] -= lij[sequential_i][j] * u_j + fa[sequential_i][j];

      _dflux_out_du[sequential_i][global_j] -= lij[sequential_i][j];
      for (const auto & dof_deriv : dfa[sequential_i][j])
        _dflux_out_du[sequential_i][dof_deriv.first] -= dof_deriv.second;

      _dflux_out_dKjk[sequential_i][index_i_to_i][j] -= 1.0 * u_j; // from the K in L = K + D

      if (sequential_j == sequential_i)
        for (dof_id_type k = 0; k < num_con_i; ++k)
          _dflux_out_dKjk[sequential_i][index_i_to_i][k] -= dDii_dKij[sequential_i][k] * u_j;
      else
        _dflux_out_dKjk[sequential_i][index_i_to_i][j] -= dDij_dKij[sequential_i][j] * u_j;
      for (dof_id_type k = 0; k < num_con_i; ++k)
        _dflux_out_dKjk[sequential_i][index_i_to_i][k] -= dFij_dKik[sequential_i][j][k];

      if (sequential_j == sequential_i)
        for (unsigned k = 0; k < con_i.size(); ++k)
        {
          const unsigned index_k_to_i =
              _connections.indexOfSequentialConnection(con_i[k], sequential_i);
          _dflux_out_dKjk[sequential_i][k][index_k_to_i] -= dDii_dKji[sequential_i][k] * u_j;
        }
      else
      {
        const unsigned index_j_to_i =
            _connections.indexOfSequentialConnection(sequential_j, sequential_i);
        _dflux_out_dKjk[sequential_i][j][index_j_to_i] -= dDij_dKji[sequential_i][j] * u_j;
      }
      for (unsigned k = 0;
           k < _connections.sequentialConnectionsToSequentialID(sequential_j).size();
           ++k)
        _dflux_out_dKjk[sequential_i][j][k] -= dFij_dKjk[sequential_i][j][k];
    }
  }
}

Real
AdvectiveFluxCalculatorBase::rPlus(dof_id_type sequential_i,
                                   std::vector<Real> & dlimited_du,
                                   std::vector<Real> & dlimited_dk) const
{
  const std::size_t num_con = _connections.sequentialConnectionsToSequentialID(sequential_i).size();
  dlimited_du.assign(num_con, 0.0);
  dlimited_dk.assign(num_con, 0.0);
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;
  std::vector<Real> dp_du;
  std::vector<Real> dp_dk;
  const Real p = PQPlusMinus(sequential_i, PQPlusMinusEnum::PPlus, dp_du, dp_dk);
  if (p == 0.0)
    // Comment after Eqn (49): if P=0 then there's no antidiffusion, so no need to remove it
    return 1.0;
  std::vector<Real> dq_du;
  std::vector<Real> dq_dk;
  const Real q = PQPlusMinus(sequential_i, PQPlusMinusEnum::QPlus, dq_du, dq_dk);

  const Real r = q / p;
  Real limited;
  Real dlimited_dr;
  limitFlux(1.0, r, limited, dlimited_dr);

  const Real p2 = std::pow(p, 2);
  for (std::size_t j = 0; j < num_con; ++j)
  {
    const Real dr_du = dq_du[j] / p - q * dp_du[j] / p2;
    const Real dr_dk = dq_dk[j] / p - q * dp_dk[j] / p2;
    dlimited_du[j] = dlimited_dr * dr_du;
    dlimited_dk[j] = dlimited_dr * dr_dk;
  }
  return limited;
}

Real
AdvectiveFluxCalculatorBase::rMinus(dof_id_type sequential_i,
                                    std::vector<Real> & dlimited_du,
                                    std::vector<Real> & dlimited_dk) const
{
  const std::size_t num_con = _connections.sequentialConnectionsToSequentialID(sequential_i).size();
  dlimited_du.assign(num_con, 0.0);
  dlimited_dk.assign(num_con, 0.0);
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;
  std::vector<Real> dp_du;
  std::vector<Real> dp_dk;
  const Real p = PQPlusMinus(sequential_i, PQPlusMinusEnum::PMinus, dp_du, dp_dk);
  if (p == 0.0)
    // Comment after Eqn (49): if P=0 then there's no antidiffusion, so no need to remove it
    return 1.0;
  std::vector<Real> dq_du;
  std::vector<Real> dq_dk;
  const Real q = PQPlusMinus(sequential_i, PQPlusMinusEnum::QMinus, dq_du, dq_dk);

  const Real r = q / p;
  Real limited;
  Real dlimited_dr;
  limitFlux(1.0, r, limited, dlimited_dr);

  const Real p2 = std::pow(p, 2);
  for (std::size_t j = 0; j < num_con; ++j)
  {
    const Real dr_du = dq_du[j] / p - q * dp_du[j] / p2;
    const Real dr_dk = dq_dk[j] / p - q * dp_dk[j] / p2;
    dlimited_du[j] = dlimited_dr * dr_du;
    dlimited_dk[j] = dlimited_dr * dr_dk;
  }
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

const std::map<dof_id_type, Real> &
AdvectiveFluxCalculatorBase::getdFluxOutdu(dof_id_type node_i) const
{
  return _dflux_out_du[_connections.sequentialID(node_i)];
}

const std::vector<std::vector<Real>> &
AdvectiveFluxCalculatorBase::getdFluxOutdKjk(dof_id_type node_i) const
{
  return _dflux_out_dKjk[_connections.sequentialID(node_i)];
}

Real
AdvectiveFluxCalculatorBase::getFluxOut(dof_id_type node_i) const
{
  return _flux_out[_connections.sequentialID(node_i)];
}

unsigned
AdvectiveFluxCalculatorBase::getValence(dof_id_type node_i) const
{
  return _valence[_connections.sequentialID(node_i)];
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
AdvectiveFluxCalculatorBase::PQPlusMinus(dof_id_type sequential_i,
                                         const PQPlusMinusEnum pq_plus_minus,
                                         std::vector<Real> & derivs,
                                         std::vector<Real> & dpqdk) const
{
  // Find the value of u at sequential_i
  const Real u_i = _u_nodal[sequential_i];

  // Connections to sequential_i
  const std::vector<dof_id_type> con_i =
      _connections.sequentialConnectionsToSequentialID(sequential_i);
  const std::size_t num_con = con_i.size();
  // The neighbor number of sequential_i to sequential_i
  const unsigned i_index_i = _connections.indexOfSequentialConnection(sequential_i, sequential_i);

  // Initialize the results
  Real result = 0.0;
  derivs.assign(num_con, 0.0);
  dpqdk.assign(num_con, 0.0);

  // Sum over all nodes connected with node_i.
  for (std::size_t j = 0; j < num_con; ++j)
  {
    const dof_id_type sequential_j = con_i[j];
    if (sequential_j == sequential_i)
      continue;
    const Real kentry = _kij[sequential_i][j];

    // Find the value of u at node_j
    const Real u_j = _u_nodal[sequential_j];
    const Real ujminusi = u_j - u_i;

    // Evaluate the i-j contribution to the result
    switch (pq_plus_minus)
    {
      case PQPlusMinusEnum::PPlus:
      {
        if (ujminusi < 0.0 && kentry < 0.0)
        {
          result += kentry * ujminusi;
          derivs[j] += kentry;
          derivs[i_index_i] -= kentry;
          dpqdk[j] += ujminusi;
        }
        break;
      }
      case PQPlusMinusEnum::PMinus:
      {
        if (ujminusi > 0.0 && kentry < 0.0)
        {
          result += kentry * ujminusi;
          derivs[j] += kentry;
          derivs[i_index_i] -= kentry;
          dpqdk[j] += ujminusi;
        }
        break;
      }
      case PQPlusMinusEnum::QPlus:
      {
        if (ujminusi > 0.0 && kentry > 0.0)
        {
          result += kentry * ujminusi;
          derivs[j] += kentry;
          derivs[i_index_i] -= kentry;
          dpqdk[j] += ujminusi;
        }
        break;
      }
      case PQPlusMinusEnum::QMinus:
      {
        if (ujminusi < 0.0 && kentry > 0.0)
        {
          result += kentry * ujminusi;
          derivs[j] += kentry;
          derivs[i_index_i] -= kentry;
          dpqdk[j] += ujminusi;
        }
        break;
      }
    }
  }

  return result;
}

void
AdvectiveFluxCalculatorBase::buildCommLists()
{
  /**
   * Build the multi-processor communication lists.
   *
   * (A) We will have to send _u_nodal information to other processors.
   * This is because although we can Evaluate Variables at all elements in
   * _fe_problem.getEvaluableElementRange(), in the PorousFlow setting
   * _u_nodal could depend on Material Properties within the elements, and
   * we can't access those Properties within the ghosted elements.
   * We don't know which processors require _u_nodal from our nodes.
   * However, we do know which nodes we require information about, and the
   * processors that can give us that information.  So we store that in
   * _nodes_to_receive.  Then we advertise to those processors that we need
   * the information.  Similarly, we will receive advertisements from the
   * other processors saying they need information from us.  We then store
   * that in _nodes_to_send.
   *
   * (B) We need to send _kij information to other processors.
   * The same strategy is followed, first building pairs_to_receive and then
   * _pairs_to_send.
   */

  // We use this (hopefully unique) message tag for all communication in this method.
  // QUESTION: If there are multiple AdvectiveFluxCalculatorBase in the input file, will this
  // work?
  Parallel::MessageTag send_tag = _communicator.get_unique_tag(4712);

  _nodes_to_receive.assign(_app.n_processors(), std::vector<dof_id_type>());
  for (const auto & elem : _fe_problem.getEvaluableElementRange())
    if (this->hasBlocks(elem->subdomain_id()))
    {
      const processor_id_type elem_pid = elem->processor_id();
      if (elem_pid != _my_pid)
        for (unsigned i = 0; i < elem->n_nodes(); ++i)
          if (std::find(_nodes_to_receive[elem_pid].begin(),
                        _nodes_to_receive[elem_pid].end(),
                        elem->node_id(i)) == _nodes_to_receive[elem_pid].end())
            _nodes_to_receive[elem_pid].push_back(elem->node_id(i));
    }

  // Now advertise that we need this information from the other processors
  std::vector<Parallel::Request> send_requests(_app.n_processors() - 1);
  unsigned sr = 0;
  for (processor_id_type pid = 0; pid < _app.n_processors(); ++pid)
    if (pid != _my_pid)
      // non-blocking send
      _communicator.send(pid, _nodes_to_receive[pid], send_requests[sr++], send_tag);

  // Now receive the adverts from all other processors
  // QUESTION: what is item_type?  Is there a better way of setting it?
  const auto item_type = libMesh::Parallel::StandardType<dof_id_type>(&(_nodes_to_receive[0][0]));
  _nodes_to_send.assign(_app.n_processors(), std::vector<dof_id_type>());
  std::vector<Parallel::Request> receive_requests(_app.n_processors() - 1);
  unsigned rr = 0;
  for (processor_id_type pid = 0; pid < _app.n_processors(); ++pid)
    if (pid != _my_pid)
    {
      // inspect incoming message
      Parallel::Status status(_communicator.probe(Parallel::any_source, send_tag));
      const auto source_pid = cast_int<processor_id_type>(status.source());
      const auto message_size = status.size(item_type);

      // resize receive buffer accordingly and receive data
      _nodes_to_send[source_pid].resize(message_size);
      _communicator.receive(
          source_pid, _nodes_to_send[source_pid], receive_requests[rr++], send_tag);
    }

  // wait until send messages are buffered before proceeding
  Parallel::wait(send_requests);
  // wait until messages are all received before proceeding, to ensure another instance of this
  // object doesn't use send_tag
  Parallel::wait(receive_requests);

  // At the moment,  _nodes_to_send and _nodes_to_receive contain global node numbers
  // It is slightly more efficient to convert to sequential node numbers
  // so that we don't have to keep doing things like
  // _u_nodal[_connections.sequentialID(_nodes_to_send[pid][i])
  // every time we send/receive
  for (processor_id_type pid = 0; pid < _app.n_processors(); ++pid)
    if (pid != _my_pid)
    {
      for (std::size_t i = 0; i < _nodes_to_send[pid].size(); ++i)
        _nodes_to_send[pid][i] = _connections.sequentialID(_nodes_to_send[pid][i]);
      for (std::size_t i = 0; i < _nodes_to_receive[pid].size(); ++i)
        _nodes_to_receive[pid][i] = _connections.sequentialID(_nodes_to_receive[pid][i]);
    }

  Parallel::MessageTag send_tag_pair = _communicator.get_unique_tag(4713);

  // Build pairs_to_receive
  // stdpairs_to_receive is created just so we can use std::find, below
  std::vector<std::vector<std::pair<dof_id_type, dof_id_type>>> stdpairs_to_receive(
      _app.n_processors(), std::vector<std::pair<dof_id_type, dof_id_type>>());
  for (const auto & elem : _fe_problem.getEvaluableElementRange())
    if (this->hasBlocks(elem->subdomain_id()))
    {
      const processor_id_type elem_pid = elem->processor_id();
      if (elem_pid != _my_pid)
        for (unsigned i = 0; i < elem->n_nodes(); ++i)
          for (unsigned j = 0; j < elem->n_nodes(); ++j)
          {
            std::pair<dof_id_type, dof_id_type> the_pair(elem->node_id(i), elem->node_id(j));
            if (std::find(stdpairs_to_receive[elem_pid].begin(),
                          stdpairs_to_receive[elem_pid].end(),
                          the_pair) == stdpairs_to_receive[elem_pid].end())
              stdpairs_to_receive[elem_pid].push_back(the_pair);
          }
    }
  // populate _pairs_to_receive
  _pairs_to_receive.assign(_app.n_processors(), std::vector<dof_id_type>());
  for (processor_id_type pid = 0; pid < _app.n_processors(); ++pid)
  {
    if (pid == _my_pid)
      continue;
    for (const auto & the_pair : stdpairs_to_receive[pid])
    {
      _pairs_to_receive[pid].push_back(the_pair.first);
      _pairs_to_receive[pid].push_back(the_pair.second);
    }
  }

  // Advertise that we need these pairs
  std::vector<Parallel::Request> send_requests_pair(_app.n_processors() - 1);
  unsigned srp = 0;
  for (processor_id_type pid = 0; pid < _app.n_processors(); ++pid)
    if (pid != _my_pid)
      // non-blocking send
      _communicator.send(pid, _pairs_to_receive[pid], send_requests_pair[srp++], send_tag_pair);

  // Receive the adverts from all other processors
  const auto pair_type = libMesh::Parallel::StandardType<dof_id_type>(&(_pairs_to_receive[0][0]));
  _pairs_to_send.assign(_app.n_processors(), std::vector<dof_id_type>());
  std::vector<Parallel::Request> receive_requests_pair(_app.n_processors() - 1);
  unsigned rrp = 0;
  for (processor_id_type pid = 0; pid < _app.n_processors(); ++pid)
    if (pid != _my_pid)
    {
      // inspect incoming message
      Parallel::Status status(_communicator.probe(Parallel::any_source, send_tag_pair));
      const auto source_pid = cast_int<processor_id_type>(status.source());
      const auto message_size = status.size(pair_type);

      // resize receive buffer accordingly and receive data
      _pairs_to_send[source_pid].resize(message_size);
      _communicator.receive(
          source_pid, _pairs_to_send[source_pid], receive_requests_pair[rrp++], send_tag_pair);
    }

  // wait until send messages are buffered before proceeding
  Parallel::wait(send_requests_pair);
  // wait until messages are all received before proceeding, to ensure another instance of this
  // object doesn't use send_tag_pair
  Parallel::wait(receive_requests_pair);

  // _pairs_to_send and _pairs_to_receive have been built using global node IDs
  // since all processors know about that.  However, using global IDs means
  // that every time we send/receive, we keep having to do things like
  // _kij[_connections.sequentialID(_pairs_to_send[pid][i])][_connections.indexOfGlobalConnection(_pairs_to_send[pid][i],
  // _pairs_to_send[pid][i + 1])] which is quite inefficient.  So:
  for (processor_id_type pid = 0; pid < _app.n_processors(); ++pid)
    if (pid != _my_pid)
    {
      for (std::size_t i = 0; i < _pairs_to_send[pid].size(); i += 2)
      {
        _pairs_to_send[pid][i + 1] = _connections.indexOfGlobalConnection(
            _pairs_to_send[pid][i], _pairs_to_send[pid][i + 1]);
        _pairs_to_send[pid][i] = _connections.sequentialID(_pairs_to_send[pid][i]);
      }
      for (std::size_t i = 0; i < _pairs_to_receive[pid].size(); i += 2)
      {
        _pairs_to_receive[pid][i + 1] = _connections.indexOfGlobalConnection(
            _pairs_to_receive[pid][i], _pairs_to_receive[pid][i + 1]);
        _pairs_to_receive[pid][i] = _connections.sequentialID(_pairs_to_receive[pid][i]);
      }
    }

  if (_my_pid == 2)
    for (unsigned pid = 0; pid < _app.n_processors(); ++pid)
      for (unsigned i = 0; i < _pairs_to_send[pid].size(); i += 2)
        Moose::err << "proc " << _my_pid << " will send kij info between nodes "
                   << _pairs_to_send[pid][i] << " and " << _pairs_to_send[pid][i + 1] << " to proc "
                   << pid << "\n";
}

void
AdvectiveFluxCalculatorBase::exchangeGhostedInfo()
{
  // Send _u_nodal to the processors that requested it
  Parallel::MessageTag send_tag = _communicator.get_unique_tag(4712);
  std::vector<Parallel::Request> send_requests(_app.n_processors() - 1);
  unsigned sr = 0;
  for (processor_id_type pid = 0; pid < _app.n_processors(); ++pid)
  {
    if (pid == _my_pid)
      continue;
    std::vector<Real> unodal_to_send;
    for (const auto & nd : _nodes_to_send[pid])
      unodal_to_send.push_back(_u_nodal[nd]);
    // non-blocking send
    _communicator.send(pid, unodal_to_send, send_requests[sr++], send_tag);
  }

  // Receive _u_nodal information from the other processors
  const auto item_type = libMesh::Parallel::StandardType<Real>(&(_u_nodal[0]));
  std::vector<Parallel::Request> receive_requests(_app.n_processors() - 1);
  unsigned rr = 0;
  for (processor_id_type pid = 0; pid < _app.n_processors(); ++pid)
  {
    if (pid == _my_pid)
      continue;
    Parallel::Status status(_communicator.probe(Parallel::any_source, send_tag));
    const auto source_pid = cast_int<processor_id_type>(status.source());
    const auto message_size = status.size(item_type);

    mooseAssert(message_size == _nodes_to_receive[source_pid].size(), "Bum");
    std::vector<Real> unodal_received(message_size);
    _communicator.receive(source_pid, unodal_received, receive_requests[rr++], send_tag);
    for (unsigned i = 0; i < message_size; ++i)
      _u_nodal[_nodes_to_receive[source_pid][i]] = unodal_received[i];
  }

  // wait until send messages are buffered before proceeding
  Parallel::wait(send_requests);
  // wait until messages are all received before proceeding, to ensure _u_nodal is properly built
  Parallel::wait(receive_requests);

  // Send _kij to the processors that requested it
  Parallel::MessageTag send_tag_pair = _communicator.get_unique_tag(4713);
  std::vector<Parallel::Request> send_requests_pair(_app.n_processors() - 1);
  unsigned srp = 0;
  for (processor_id_type pid = 0; pid < _app.n_processors(); ++pid)
  {
    if (pid == _my_pid)
      continue;
    std::vector<Real> kij_to_send;
    for (std::size_t i = 0; i < _pairs_to_send[pid].size(); i += 2)
      kij_to_send.push_back(_kij[_pairs_to_send[pid][i]][_pairs_to_send[pid][i + 1]]);
    // non-blocking send
    _communicator.send(pid, kij_to_send, send_requests_pair[srp++], send_tag_pair);
  }

  // Receive _kij information from the other processors
  const auto kij_type = libMesh::Parallel::StandardType<Real>(&(_kij[0][0]));
  std::vector<Parallel::Request> receive_requests_pair(_app.n_processors() - 1);
  unsigned rrp = 0;
  for (processor_id_type pid = 0; pid < _app.n_processors(); ++pid)
  {
    if (pid == _my_pid)
      continue;
    Parallel::Status status(_communicator.probe(Parallel::any_source, send_tag_pair));
    const auto source_pid = cast_int<processor_id_type>(status.source());
    const auto message_size = status.size(kij_type);

    mooseAssert(message_size == _pairs_to_receive[source_pid].size() / 2, "Bum2");
    std::vector<Real> kij_received(message_size);
    _communicator.receive(source_pid, kij_received, receive_requests_pair[rrp++], send_tag_pair);
    for (unsigned i = 0; i < message_size; ++i)
      _kij[_pairs_to_receive[source_pid][2 * i]][_pairs_to_receive[source_pid][2 * i + 1]] +=
          kij_received[i];
  }

  // wait until send messages are buffered before proceeding
  Parallel::wait(send_requests_pair);
  // wait until messages are all received before proceeding, to ensure _kij is properly built
  Parallel::wait(receive_requests_pair);
}
