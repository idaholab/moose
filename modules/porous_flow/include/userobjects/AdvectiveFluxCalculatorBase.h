//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "PorousFlowConnectedNodes.h"

/**
 * Base class to compute Advective fluxes.  Specifically,
 * computes K_ij, D_ij, L_ij, R+, R-, f^a_ij detailed in
 * D Kuzmin and S Turek "High-resolution FEM-TVD schemes based on a fully multidimensional flux
 * limiter" Journal of Computational Physics 198 (2004) 131-158
 * Then combines the results to produce the residual and Jacobian contributions that
 * may be used by Kernels
 *
 * K_ij is a measure of flux from node i to node j
 * D_ij is a diffusion matrix that stabilizes K_ij (K+D has the LED property)
 * L = K + D
 * R^+_i and R^-_i and f^a_{ij} quantify how much antidiffusion to allow around node i
 */
class AdvectiveFluxCalculatorBase : public ElementUserObject
{
public:
  static InputParameters validParams();

  AdvectiveFluxCalculatorBase(const InputParameters & parameters);

  virtual void timestepSetup() override;

  virtual void meshChanged() override;

  virtual void initialize() override;

  virtual void threadJoin(const UserObject & uo) override;

  virtual void finalize() override;

  virtual void execute() override;

  /**
   * This is called by multiple times in  execute() in a double loop over _current_elem's nodes
   * (local_i and local_j) nested in a loop over each of _current_elem's quadpoints (qp). It is used
   * to compute _kij and its derivatives
   * @param global_i global node id corresponding to the local node local_i
   * @param global_j global node id corresponding to the local node local_j
   * @param local_i local node number of the _current_elem
   * @param local_j local node number of the _current_elem
   * @param qp quadpoint number of the _current_elem
   */
  virtual void executeOnElement(
      dof_id_type global_i, dof_id_type global_j, unsigned local_i, unsigned local_j, unsigned qp);

  /**
   * Returns the flux out of lobal node id
   * @param node_i id of node
   * @return advective flux out of node after applying the KT procedure
   */
  Real getFluxOut(dof_id_type node_i) const;

  /**
   * Returns r where r[j] = d(flux out of global node i)/du(global node j) used in Jacobian
   * computations
   * @param node_i global id of node
   * @return the derivatives (after applying the KT procedure)
   */
  const std::map<dof_id_type, Real> & getdFluxOutdu(dof_id_type node_i) const;

  /**
   * Returns r where r[j][k] = d(flux out of global node i)/dK[connected node j][connected node k]
   * used in Jacobian computations
   * @param node_i global id of node
   * @return the derivatives (after applying the KT procedure)
   */
  const std::vector<std::vector<Real>> & getdFluxOutdKjk(dof_id_type node_i) const;

  /**
   * Returns the valence of the global node i
   * Valence is the number of times the node is encountered in a loop over elements (that have
   * appropriate subdomain_id, if the user has employed the "blocks=" parameter) seen by this
   * processor (including ghosted elements)
   * @param node_i gloal id of i^th node
   * @return valence of the node
   */
  unsigned getValence(dof_id_type node_i) const;

protected:
  /**
   * When using multiple processors, other processors will compute:
   *  - u_nodal for nodes that we don't "own", but that we need when doing the stabilization
   *  - k_ij for node pairs that we don't "own", and contributions to node pairs that we do "own"
   * (on the boundary of our set of elements), that are used in the stabilization
   * This method builds _nodes_to_receive and _pairs_to_receive that describe which processors we
   * are going to receive this info from, and similarly it builds _nodes_to_send and _pairs_to_send.
   */
  virtual void buildCommLists();

  /**
   * Sends and receives multi-processor information regarding u_nodal and k_ij.
   * See buildCommLists for some more explanation.
   */
  virtual void exchangeGhostedInfo();

  /**
   * Computes the transfer velocity between current node i and current node j
   * at the current qp in the current element (_current_elem).
   * For instance, (_grad_phi[i][qp] * _velocity) * _phi[j][qp];
   * @param i node number in the current element
   * @param j node number in the current element
   * @param qp quadpoint number in the current element
   */
  virtual Real computeVelocity(unsigned i, unsigned j, unsigned qp) const = 0;

  /**
   * Computes the value of u at the local node id of the current element (_current_elem)
   * @param i local node id of the current element
   */
  virtual Real computeU(unsigned i) const = 0;

  /// whether _kij, etc, need to be sized appropriately (and valence recomputed) at the start of the timestep
  bool _resizing_needed;

  /**
   * flux limiter, L, on Page 135 of Kuzmin and Turek
   * @param a KT's "a" parameter
   * @param b KT's "b" parameter
   * @param limited[out] The value of the flux limiter, L
   * @param dlimited_db[out] The derivative dL/db
   */
  void limitFlux(Real a, Real b, Real & limited, Real & dlimited_db) const;

  /**
   * Returns the value of R_{i}^{+}, Eqn (49) of KT
   * @param node_i nodal id
   * @param dlimited_du[out] dlimited_du[j] = d(R_{sequential_i}^{+})/du[sequential_j].  Here
   * sequential_j is the j^th connection to sequential_i
   * @param dlimited_dk[out] dlimited_dk[j] = d(R_{sequential_i}^{+})/d(K[sequential_i][j]).  Note
   * Derivatives w.r.t. K[l][m] with l!=i are zero
   */
  Real rPlus(dof_id_type sequential_i,
             std::vector<Real> & dlimited_du,
             std::vector<Real> & dlimited_dk) const;

  /**
   * Returns the value of R_{i}^{-}, Eqn (49) of KT
   * @param sequential_i Sequential nodal ID
   * @param dlimited_du[out] dlimited_du[j] = d(R_{sequential_i}^{-})/du[sequential_j].  Here
   * sequential_j is the j^th connection to sequential_i
   * @param dlimited_dk[out] dlimited_dk[j] = d(R_{sequential_i}^{-})/d(K[sequential_i][j]).  Note
   * Derivatives w.r.t. K[l][m] with l!=i are zero
   */
  Real rMinus(dof_id_type sequential_i,
              std::vector<Real> & dlimited_du,
              std::vector<Real> & dlimited_dk) const;

  /**
   * Determines Flux Limiter type (Page 135 of Kuzmin and Turek)
   * "None" means that limitFlux=0 always, which implies zero antidiffusion will be added
   */
  const enum class FluxLimiterTypeEnum { MinMod, VanLeer, MC, superbee, None } _flux_limiter_type;

  /// Kuzmin-Turek K_ij matrix.  Along with R+ and R-, this is the key quantity computed
  /// by this UserObject.
  /// _kij[i][j] = k_ij corresponding to the i-j node pair.
  /// Here i is a sequential node numbers according to the _connections object,
  /// and j represents the j^th node connected to i according to the _connections object.
  std::vector<std::vector<Real>> _kij;

  /// _flux_out[i] = flux of "heat" from sequential node i
  std::vector<Real> _flux_out;

  /// _dflux_out_du[i][j] = d(flux_out[i])/d(u[j]).
  /// Here i is a sequential node number according to the _connections object, and j (global ID) must be connected to i, or to a node that is connected to i.
  std::vector<std::map<dof_id_type, Real>> _dflux_out_du;

  /// _dflux_out_dKjk[sequential_i][j][k] = d(flux_out[sequential_i])/d(K[j][k]).
  /// Here sequential_i is a sequential node number according to the _connections object,
  /// and j represents the j^th node connected to i according to the _connections object,
  /// and k represents the k^th node connected to j according to the _connections object.
  /// Here j must be connected to i (this does include (the sequential version of j) == i), and k must be connected to j (this does include (the sequential version of k) = i and (the sequential version of k) == (sequential version of j)))
  std::vector<std::vector<std::vector<Real>>> _dflux_out_dKjk;

  /// _valence[i] = number of times, in a loop over elements seen  by this processor
  /// (viz, including ghost elements) and are part of the block-restricted blocks of this
  /// UserObject, that the sequential node i is encountered
  std::vector<unsigned> _valence;

  /// _u_nodal[i] = value of _u at sequential node number i
  std::vector<Real> _u_nodal;

  /// _u_nodal_computed_by_thread(i) = true if _u_nodal[i] has been computed in execute() by the thread on this processor
  std::vector<bool> _u_nodal_computed_by_thread;

  /// Holds the sequential and global nodal IDs, and info regarding mesh connections between them
  PorousFlowConnectedNodes _connections;

  /// Number of nodes held by the _connections object
  std::size_t _number_of_nodes;

  /// processor ID of this object
  processor_id_type _my_pid;

  /**
   * _nodes_to_receive[proc_id] = list of sequential nodal IDs.  proc_id will send us _u_nodal at
   * those nodes. _nodes_to_receive is built (in buildCommLists()) using global node IDs, but
   * after construction, a translation to sequential node IDs is made, for efficiency.
   * The result is: we will receive _u_nodal[_nodes_to_receive[proc_id][i]] from proc_id
   */
  std::map<processor_id_type, std::vector<dof_id_type>> _nodes_to_receive;

  /**
   * _nodes_to_send[proc_id] = list of sequential nodal IDs.  We will send _u_nodal at those nodes
   * to proc_id _nodes_to_send is built (in buildCommLists()) using global node IDs, but after
   * construction, a translation to sequential node IDs is made, for efficiency
   * The result is: we will send _u_nodal[_nodes_to_receive[proc_id][i]] to proc_id
   */
  std::map<processor_id_type, std::vector<dof_id_type>> _nodes_to_send;

  /**
   * _pairs_to_receive[proc_id] indicates the k(i, j) pairs that will be sent to us from proc_id
   * _pairs_to_receive is first built (in buildCommLists()) using global node IDs, but after
   * construction, a translation to sequential node IDs and the index of connections is
   * performed, for efficiency.  The result is we will receive:
   * _kij[_pairs_to_receive[proc_id][i].first][_pairs_to_receive[proc_id][i].second] from proc_id
   */
  std::map<processor_id_type, std::vector<std::pair<dof_id_type, dof_id_type>>> _pairs_to_receive;

  /**
   * _pairs_to_send[proc_id] indicates the k(i, j) pairs that we will send to proc_id
   * _pairs_to_send is first built (in buildCommLists()) using global node IDs, but after
   * construction, a translation to sequential node IDs and the index of connections is
   * performed, for efficiency.  The result is we will send:
   * _kij[_pairs_to_send[proc_id][i].first][_pairs_to_send[proc_id][i+1].second] to proc_id
   */
  std::map<processor_id_type, std::vector<std::pair<dof_id_type, dof_id_type>>> _pairs_to_send;

  /**
   * A mooseWarning is issued if mb_wasted = (_connections.sizeSequential() -
   * _connections.numNodes()) * 4 / 1048576 > _allowable_MB_wastage.  The _connections object uses
   * sequential node numbering for computational efficiency, but this leads to memory being used
   * inefficiently: the number of megabytes wasted is mb_wasted.
   */
  const Real _allowable_MB_wastage;

  /// Signals to the PQPlusMinus method what should be computed
  enum class PQPlusMinusEnum
  {
    PPlus,
    PMinus,
    QPlus,
    QMinus
  };

  /**
   * Returns the value of P_{i}^{+}, P_{i}^{-}, Q_{i}^{+} or Q_{i}^{-} (depending on pq_plus_minus)
   * which are defined in Eqns (47) and (48) of KT
   * @param sequential_i sequential nodal ID
   * @param pq_plus_minus indicates whether P_{i}^{+}, P_{i}^{-}, Q_{i}^{+} or Q_{i}^{-} should be
   * returned
   * @param derivs[out] derivs[j] = d(result)/d(u[sequential_j]).  Here sequential_j is the j^th
   * connection to sequential_i
   * @param dpq_dk[out] dpq_dk[j] = d(result)/d(K[node_i][j]).  Here j indexes a connection to
   * sequential_i.  Recall that d(result)/d(K[l][m]) are zero unless l=sequential_i
   */
  Real PQPlusMinus(dof_id_type sequential_i,
                   const PQPlusMinusEnum pq_plus_minus,
                   std::vector<Real> & derivs,
                   std::vector<Real> & dpq_dk) const;

  /**
   * Clears the_map, then, using _kij, constructs the_map so that
   * the_map[node_id] = 0.0 for all node_id connected with node_i
   * @param[out] the_map the map to be zeroed appropriately
   * @param[in] node_i nodal id
   */
  void zeroedConnection(std::map<dof_id_type, Real> & the_map, dof_id_type node_i) const;

  /// Vectors used in finalize()
  std::vector<std::vector<Real>> _dij;
  /// dDij_dKij[i][j] = d(D[i][j])/d(K[i][j]) for i!=j
  std::vector<std::vector<Real>> _dDij_dKij;
  /// dDij_dKji[i][j] = d(D[i][j])/d(K[j][i]) for i!=j
  std::vector<std::vector<Real>> _dDij_dKji;
  /// dDii_dKij[i][j] = d(D[i][i])/d(K[i][j])
  std::vector<std::vector<Real>> _dDii_dKij;
  /// dDii_dKji[i][j] = d(D[i][i])/d(K[j][i])
  std::vector<std::vector<Real>> _dDii_dKji;

  std::vector<std::vector<Real>> _lij;
  std::vector<Real> _rP;
  std::vector<Real> _rM;

  /// drP[i][j] = d(rP[i])/d(u[j]). Here j indexes the j^th node connected to i
  std::vector<std::vector<Real>> _drP;
  /// drM[i][j] = d(rM[i])/d(u[j]). Here j indexes the j^th node connected to i
  std::vector<std::vector<Real>> _drM;
  /// drP_dk[i][j] = d(rP[i])/d(K[i][j]).  Here j indexes the j^th node connected to i
  std::vector<std::vector<Real>> _drP_dk;
  /// drM_dk[i][j] = d(rM[i])/d(K[i][j]).  Here j indexes the j^th node connected to i
  std::vector<std::vector<Real>> _drM_dk;

  /// fa[sequential_i][j]  sequential_j is the j^th connection to sequential_i
  std::vector<std::vector<Real>> _fa;
  /// dfa[sequential_i][j][global_k] = d(fa[sequential_i][j])/du[global_k].
  /// Here global_k can be a neighbor to sequential_i or a neighbour to
  /// sequential_j (sequential_j is the j^th connection to sequential_i)
  std::vector<std::vector<std::map<dof_id_type, Real>>> _dfa;
  /// dFij_dKik[sequential_i][j][k] =
  /// d(fa[sequential_i][j])/d(K[sequential_i][k]).  Here j denotes
  /// the j^th connection to sequential_i, while k denotes the k^th
  /// connection to sequential_i
  std::vector<std::vector<std::vector<Real>>> _dFij_dKik;
  /// dFij_dKjk[sequential_i][j][k] =
  /// d(fa[sequential_i][j])/d(K[sequential_j][k]).  Here sequential_j is
  /// the j^th connection to sequential_i, and k denotes the k^th connection
  /// to sequential_j (this will include sequential_i itself)
  std::vector<std::vector<std::vector<Real>>> _dFij_dKjk;
};
