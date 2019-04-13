//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableFE.h"
#include <typeinfo>
#include "TimeIntegrator.h"
#include "NonlinearSystemBase.h"
#include "DisplacedSystem.h"
#include "Assembly.h"

template <typename OutputType>
MooseVariableFE<OutputType>::MooseVariableFE(unsigned int var_num,
                                             const FEType & fe_type,
                                             SystemBase & sys,
                                             Assembly & assembly,
                                             Moose::VarKindType var_kind,
                                             THREAD_ID tid)
  : MooseVariableFEBase(var_num, fe_type, sys, var_kind, tid),
    _displaced(dynamic_cast<DisplacedSystem *>(&sys) ? true : false),
    _assembly(assembly),
    _qrule(_assembly.qRule()),
    _qrule_face(_assembly.qRuleFace()),
    _qrule_neighbor(_assembly.qRuleNeighbor()),
    _elem(_assembly.elem()),
    _current_side(_assembly.side()),
    _neighbor(_assembly.neighbor()),
    _need_u_old(false),
    _need_u_older(false),
    _need_u_previous_nl(false),
    _need_u_dot(false),
    _need_ad_u_dot(false),
    _need_u_dotdot(false),
    _need_u_dot_old(false),
    _need_u_dotdot_old(false),
    _need_du_dot_du(false),
    _need_du_dotdot_du(false),
    _need_grad_old(false),
    _need_grad_older(false),
    _need_grad_previous_nl(false),
    _need_grad_dot(false),
    _need_grad_dotdot(false),
    _need_second(false),
    _need_second_old(false),
    _need_second_older(false),
    _need_second_previous_nl(false),
    _need_curl(false),
    _need_curl_old(false),
    _need_curl_older(false),
    _need_ad(false),
    _need_ad_u(false),
    _need_ad_grad_u(false),
    _need_ad_second_u(false),
    _need_neighbor_ad(false),
    _need_neighbor_ad_u(false),
    _need_neighbor_ad_grad_u(false),
    _need_neighbor_ad_second_u(false),
    _need_u_old_neighbor(false),
    _need_u_older_neighbor(false),
    _need_u_previous_nl_neighbor(false),
    _need_u_dot_neighbor(false),
    _need_neighbor_ad_u_dot(false),
    _need_u_dotdot_neighbor(false),
    _need_u_dot_old_neighbor(false),
    _need_u_dotdot_old_neighbor(false),
    _need_du_dot_du_neighbor(false),
    _need_du_dotdot_du_neighbor(false),
    _need_grad_old_neighbor(false),
    _need_grad_older_neighbor(false),
    _need_grad_previous_nl_neighbor(false),
    _need_grad_neighbor_dot(false),
    _need_grad_neighbor_dotdot(false),
    _need_second_neighbor(false),
    _need_second_old_neighbor(false),
    _need_second_older_neighbor(false),
    _need_second_previous_nl_neighbor(false),
    _need_curl_neighbor(false),
    _need_curl_old_neighbor(false),
    _need_curl_older_neighbor(false),
    _need_dof_values(false),
    _need_dof_values_old(false),
    _need_dof_values_older(false),
    _need_dof_values_previous_nl(false),
    _need_dof_values_dot(false),
    _need_dof_values_dotdot(false),
    _need_dof_values_dot_old(false),
    _need_dof_values_dotdot_old(false),
    _need_dof_du_dot_du(false),
    _need_dof_du_dotdot_du(false),
    _need_dof_values_neighbor(false),
    _need_dof_values_old_neighbor(false),
    _need_dof_values_older_neighbor(false),
    _need_dof_values_previous_nl_neighbor(false),
    _need_dof_values_dot_neighbor(false),
    _need_dof_values_dotdot_neighbor(false),
    _need_dof_values_dot_old_neighbor(false),
    _need_dof_values_dotdot_old_neighbor(false),
    _need_dof_du_dot_du_neighbor(false),
    _need_dof_du_dotdot_du_neighbor(false),
    _normals(_assembly.normals()),
    _is_nodal(true),
    _has_dof_indices(false),
    _neighbor_has_dof_indices(false),
    _has_dof_values(false),
    _has_dof_values_neighbor(false),
    _node(_assembly.node()),
    _node_neighbor(_assembly.nodeNeighbor()),
    _phi(_assembly.fePhi<OutputType>(_fe_type)),
    _grad_phi(_assembly.feGradPhi<OutputType>(_fe_type)),
    _phi_face(_assembly.fePhiFace<OutputType>(_fe_type)),
    _grad_phi_face(_assembly.feGradPhiFace<OutputType>(_fe_type)),
    _phi_neighbor(_assembly.fePhiNeighbor<OutputType>(_fe_type)),
    _grad_phi_neighbor(_assembly.feGradPhiNeighbor<OutputType>(_fe_type)),
    _phi_face_neighbor(_assembly.fePhiFaceNeighbor<OutputType>(_fe_type)),
    _grad_phi_face_neighbor(_assembly.feGradPhiFaceNeighbor<OutputType>(_fe_type)),
    _ad_grad_phi(_assembly.feADGradPhi<OutputType>(_fe_type)),
    _ad_grad_phi_face(_assembly.feADGradPhiFace<OutputType>(_fe_type)),
    _ad_u(),
    _ad_grad_u(),
    _ad_second_u(),
    _ad_dof_values(),
    _neighbor_ad_u(),
    _neighbor_ad_grad_u(),
    _neighbor_ad_second_u(),
    _neighbor_ad_dof_values(),
    _ad_zero(0),
    _time_integrator(nullptr)
{
  // FIXME: continuity of FE type seems equivalent with the definition of nodal variables.
  //        Continuity does not depend on the FE dimension, so we just pass in a valid dimension.
  if (_fe_type.family == NEDELEC_ONE || _fe_type.family == LAGRANGE_VEC)
    _continuity = _assembly.getVectorFE(_fe_type, _sys.mesh().dimension())->get_continuity();
  else
    _continuity = _assembly.getFE(_fe_type, _sys.mesh().dimension())->get_continuity();

  _is_nodal = (_continuity == C_ZERO || _continuity == C_ONE);

  auto num_vector_tags = _sys.subproblem().numVectorTags();

  _vector_tags_dof_u.resize(num_vector_tags);
  _need_vector_tag_dof_u.resize(num_vector_tags);

  _need_vector_tag_u.resize(num_vector_tags);
  _vector_tag_u.resize(num_vector_tags);

  auto num_matrix_tags = _sys.subproblem().numMatrixTags();

  _matrix_tags_dof_u.resize(num_matrix_tags);
  _need_matrix_tag_dof_u.resize(num_matrix_tags);

  _need_matrix_tag_u.resize(num_matrix_tags);
  _matrix_tag_u.resize(num_matrix_tags);

  _time_integrator = _sys.getTimeIntegrator();
}

template <typename OutputType>
MooseVariableFE<OutputType>::~MooseVariableFE()
{
  _dof_values.release();
  _dof_values_old.release();
  _dof_values_older.release();
  _dof_values_previous_nl.release();
  _dof_values_dot.release();
  _dof_values_dotdot.release();
  _dof_values_dot_old.release();
  _dof_values_dotdot_old.release();
  _dof_du_dot_du.release();
  _dof_du_dotdot_du.release();

  _dof_values_neighbor.release();
  _dof_values_old_neighbor.release();
  _dof_values_older_neighbor.release();
  _dof_values_previous_nl_neighbor.release();
  _dof_values_dot_neighbor.release();
  _dof_values_dotdot_neighbor.release();
  _dof_values_dot_old_neighbor.release();
  _dof_values_dotdot_old_neighbor.release();
  _dof_du_dot_du_neighbor.release();
  _dof_du_dotdot_du_neighbor.release();

  for (auto & dof_u : _vector_tags_dof_u)
    dof_u.release();

  _vector_tags_dof_u.clear();

  for (auto & dof_u : _matrix_tags_dof_u)
    dof_u.release();

  _matrix_tags_dof_u.clear();

  for (auto & tag_u : _vector_tag_u)
    tag_u.release();

  _vector_tag_u.clear();

  for (auto & tag_u : _matrix_tag_u)
    tag_u.release();

  _matrix_tag_u.clear();

  _u.release();
  _u_old.release();
  _u_older.release();
  _u_previous_nl.release();

  _grad_u.release();
  _grad_u_old.release();
  _grad_u_older.release();
  _grad_u_older.release();
  _grad_u_previous_nl.release();
  _grad_u_dot.release();
  _grad_u_dotdot.release();

  _second_u.release();
  _second_u_old.release();
  _second_u_older.release();
  _second_u_previous_nl.release();

  _curl_u.release();
  _curl_u_old.release();
  _curl_u_older.release();

  _ad_u.release();
  _ad_grad_u.release();
  _ad_second_u.release();

  _u_dot.release();
  _u_dot_neighbor.release();

  _u_dotdot.release();
  _u_dotdot_bak.release();
  _u_dotdot_neighbor.release();
  _u_dotdot_bak_neighbor.release();

  _u_dot_old.release();
  _u_dot_old_bak.release();
  _u_dot_old_neighbor.release();
  _u_dot_old_bak_neighbor.release();

  _u_dotdot_old.release();
  _u_dotdot_old_bak.release();
  _u_dotdot_old_neighbor.release();
  _u_dotdot_old_bak_neighbor.release();

  _du_dot_du.release();
  _du_dot_du_neighbor.release();

  _du_dotdot_du.release();
  _du_dotdot_du_bak.release();
  _du_dotdot_du_neighbor.release();
  _du_dotdot_du_bak_neighbor.release();

  _increment.release();

  _u_neighbor.release();
  _u_old_neighbor.release();
  _u_older_neighbor.release();
  _u_previous_nl_neighbor.release();

  _grad_u_neighbor.release();
  _grad_u_old_neighbor.release();
  _grad_u_older_neighbor.release();
  _grad_u_previous_nl_neighbor.release();
  _grad_u_neighbor_dot.release();
  _grad_u_neighbor_dotdot.release();

  _second_u_neighbor.release();
  _second_u_old_neighbor.release();
  _second_u_older_neighbor.release();
  _second_u_previous_nl_neighbor.release();

  _curl_u_neighbor.release();
  _curl_u_old_neighbor.release();
  _curl_u_older_neighbor.release();

  _ad_u.release();
  _ad_grad_u.release();
  _ad_second_u.release();
  _ad_dof_values.release();
  _ad_dofs_dot.release();
  _ad_u_dot.release();

  _neighbor_ad_u.release();
  _neighbor_ad_grad_u.release();
  _neighbor_ad_second_u.release();
  _neighbor_ad_dof_values.release();
  _neighbor_ad_dofs_dot.release();
  _neighbor_ad_u_dot.release();
}

template <typename OutputType>
const std::set<SubdomainID> &
MooseVariableFE<OutputType>::activeSubdomains() const
{
  return _sys.system().variable(_var_num).active_subdomains();
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::activeOnSubdomain(SubdomainID subdomain) const
{
  return _sys.system().variable(_var_num).active_on_subdomain(subdomain);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::clearDofIndices()
{
  _dof_indices.clear();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::prepare()
{
  _dof_map.dof_indices(_elem, _dof_indices, _var_num);
  _has_dof_values = false;
  _has_dof_values_neighbor = false;

  // FIXME: remove this when the Richard's module is migrated to use the new NodalCoupleable
  // interface.
  if (_dof_indices.size() > 0)
    _has_dof_indices = true;
  else
    _has_dof_indices = false;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::prepareNeighbor()
{
  _dof_map.dof_indices(_neighbor, _dof_indices_neighbor, _var_num);
  _has_dof_values = false;
  _has_dof_values_neighbor = false;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::prepareAux()
{
  _has_dof_values = false;
  _has_dof_values_neighbor = false;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::reinitNode()
{
  if (size_t n_dofs = _node->n_dofs(_sys.number(), _var_num))
  {
    _dof_indices.resize(n_dofs);
    for (size_t i = 0; i < n_dofs; ++i)
      _dof_indices[i] = _node->dof_number(_sys.number(), _var_num, i);
    // For standard variables. _nodal_dof_index is retrieved by nodalDofIndex() which is used in
    // NodalBC for example
    _nodal_dof_index = _dof_indices[0];
    _has_dof_indices = true;
  }
  else
  {
    _dof_indices.clear(); // Clear these so Assembly doesn't think there's dofs here
    _has_dof_indices = false;
  }
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::reinitAux()
{
  /* FIXME: this method is only for elemental auxiliary variables, so
   * we may want to rename it */
  _dof_map.dof_indices(_elem, _dof_indices, _var_num);
  if (_elem->n_dofs(_sys.number(), _var_num) > 0)
  {
    // FIXME: check if the following is equivalent with '_nodal_dof_index = _dof_indices[0];'?
    _nodal_dof_index = _elem->dof_number(_sys.number(), _var_num, 0);

    fetchDoFValues();

    for (auto & dof_u : _vector_tags_dof_u)
      dof_u.resize(_dof_indices.size());

    for (auto & dof_u : _matrix_tags_dof_u)
      dof_u.resize(_dof_indices.size());

    _has_dof_indices = true;
  }
  else
    _has_dof_indices = false;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::reinitAuxNeighbor()
{
  if (_neighbor)
  {
    _dof_map.dof_indices(_neighbor, _dof_indices_neighbor, _var_num);
    if (_neighbor->n_dofs(_sys.number(), _var_num) > 0)
    {
      _nodal_dof_index_neighbor = _neighbor->dof_number(_sys.number(), _var_num, 0);

      fetchDoFValuesNeighbor();

      _neighbor_has_dof_indices = true;
    }
    else
      _neighbor_has_dof_indices = false;
  }
  else
    _neighbor_has_dof_indices = false;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::reinitNodes(const std::vector<dof_id_type> & nodes)
{
  _dof_indices.clear();
  for (const auto & node_id : nodes)
  {
    Node * nd = _subproblem.mesh().getMesh().query_node_ptr(node_id);
    if (nd && (_subproblem.mesh().isSemiLocal(nd)))
    {
      if (nd->n_dofs(_sys.number(), _var_num) > 0)
      {
        dof_id_type dof = nd->dof_number(_sys.number(), _var_num, 0);
        _dof_indices.push_back(dof);
      }
    }
  }

  if (_dof_indices.size() > 0)
    _has_dof_indices = true;
  else
    _has_dof_indices = false;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::reinitNodesNeighbor(const std::vector<dof_id_type> & nodes)
{
  _dof_indices_neighbor.clear();
  for (const auto & node_id : nodes)
  {
    Node * nd = _subproblem.mesh().getMesh().query_node_ptr(node_id);
    if (nd && (_subproblem.mesh().isSemiLocal(nd)))
    {
      if (nd->n_dofs(_sys.number(), _var_num) > 0)
      {
        dof_id_type dof = nd->dof_number(_sys.number(), _var_num, 0);
        _dof_indices_neighbor.push_back(dof);
      }
    }
  }

  if (_dof_indices_neighbor.size() > 0)
    _neighbor_has_dof_indices = true;
  else
    _neighbor_has_dof_indices = false;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::getDofIndices(const Elem * elem,
                                           std::vector<dof_id_type> & dof_indices)
{
  _dof_map.dof_indices(elem, dof_indices, _var_num);
}

template <typename OutputType>
Number
MooseVariableFE<OutputType>::getNodalValue(const Node & node)
{
  mooseAssert(_subproblem.mesh().isSemiLocal(const_cast<Node *>(&node)), "Node is not Semilocal");

  // Make sure that the node has DOFs
  /* Note, this is a reproduction of an assert within libMesh::Node::dof_number, this is done to
   * produce a better error (see misc/check_error.node_value_off_block) */
  mooseAssert(node.n_dofs(_sys.number(), _var_num) > 0,
              "Node " << node.id() << " does not contain any dofs for the "
                      << _sys.system().variable_name(_var_num) << " variable");

  dof_id_type dof = node.dof_number(_sys.number(), _var_num, 0);

  return (*_sys.currentSolution())(dof);
}

template <typename OutputType>
Number
MooseVariableFE<OutputType>::getNodalValueOld(const Node & node)
{
  mooseAssert(_subproblem.mesh().isSemiLocal(const_cast<Node *>(&node)), "Node is not Semilocal");

  // Make sure that the node has DOFs
  /* Note, this is a reproduction of an assert within libMesh::Node::dof_number, this is done to
   * produce a better error (see misc/check_error.node_value_off_block) */
  mooseAssert(node.n_dofs(_sys.number(), _var_num) > 0,
              "Node " << node.id() << " does not contain any dofs for the "
                      << _sys.system().variable_name(_var_num) << " variable");

  dof_id_type dof = node.dof_number(_sys.number(), _var_num, 0);
  return _sys.solutionOld()(dof);
}

template <typename OutputType>
Number
MooseVariableFE<OutputType>::getNodalValueOlder(const Node & node)
{
  mooseAssert(_subproblem.mesh().isSemiLocal(const_cast<Node *>(&node)), "Node is not Semilocal");

  // Make sure that the node has DOFs
  /* Note, this is a reproduction of an assert within libMesh::Node::dof_number, this is done to
   * produce a better error (see misc/check_error.node_value_off_block) */
  mooseAssert(node.n_dofs(_sys.number(), _var_num) > 0,
              "Node " << node.id() << " does not contain any dofs for the "
                      << _sys.system().variable_name(_var_num) << " variable");

  dof_id_type dof = node.dof_number(_sys.number(), _var_num, 0);
  return _sys.solutionOlder()(dof);
}

template <typename OutputType>
Number
MooseVariableFE<OutputType>::getElementalValue(const Elem * elem, unsigned int idx) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  return (*_sys.currentSolution())(dof_indices[idx]);
}

template <typename OutputType>
Number
MooseVariableFE<OutputType>::getElementalValueOld(const Elem * elem, unsigned int idx) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  return _sys.solutionOld()(dof_indices[idx]);
}

template <typename OutputType>
Number
MooseVariableFE<OutputType>::getElementalValueOlder(const Elem * elem, unsigned int idx) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  return _sys.solutionOlder()(dof_indices[idx]);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::insert(NumericVector<Number> & residual)
{
  if (_has_dof_values)
    residual.insert(&_dof_values[0], _dof_indices);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::add(NumericVector<Number> & residual)
{
  if (_has_dof_values)
    residual.add_vector(&_dof_values[0], _dof_indices);
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValue()
{
  mooseDeprecated("Use dofValues instead of dofValue");
  return dofValues();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValues()
{
  _need_dof_values = true;
  return _dof_values;
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesOld()
{
  _need_dof_values_old = true;
  return _dof_values_old;
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesOlder()
{
  _need_dof_values_older = true;
  return _dof_values_older;
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesPreviousNL()
{
  _need_dof_values_previous_nl = true;
  return _dof_values_previous_nl;
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesNeighbor()
{
  _need_dof_values_neighbor = true;
  return _dof_values_neighbor;
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesOldNeighbor()
{
  _need_dof_values_old_neighbor = true;
  return _dof_values_old_neighbor;
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesOlderNeighbor()
{
  _need_dof_values_older_neighbor = true;
  return _dof_values_older_neighbor;
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesPreviousNLNeighbor()
{
  _need_dof_values_previous_nl_neighbor = true;
  return _dof_values_previous_nl_neighbor;
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDot()
{
  if (_sys.solutionUDot())
  {
    _need_dof_values_dot = true;
    return _dof_values_dot;
  }
  else
    mooseError("MooseVariableFE: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase before requesting `u_dot`.");
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotDot()
{
  if (_sys.solutionUDotDot())
  {
    _need_dof_values_dotdot = true;
    return _dof_values_dotdot;
  }
  else
    mooseError("MooseVariableFE: Second time derivative of solution (`u_dotdot`) is not stored. "
               "Please set uDotDotRequested() to true in FEProblemBase before requesting "
               "`u_dotdot`.");
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotOld()
{
  if (_sys.solutionUDotOld())
  {
    _need_dof_values_dot_old = true;
    return _dof_values_dot_old;
  }
  else
    mooseError("MooseVariableFE: Old time derivative of solution (`u_dot_old`) is not stored. "
               "Please set uDotOldRequested() to true in FEProblemBase before requesting "
               "`u_dot_old`.");
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotDotOld()
{
  if (_sys.solutionUDotDotOld())
  {
    _need_dof_values_dotdot_old = true;
    return _dof_values_dotdot_old;
  }
  else
    mooseError("MooseVariableFE: Old second time derivative of solution (`u_dotdot_old`) is not "
               "stored. Please set uDotDotOldRequested() to true in FEProblemBase before "
               "requesting `u_dotdot_old`.");
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotNeighbor()
{
  if (_sys.solutionUDot())
  {
    _need_dof_values_dot_neighbor = true;
    return _dof_values_dot_neighbor;
  }
  else
    mooseError("MooseVariableFE: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase before requesting `u_dot`.");
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotDotNeighbor()
{
  if (_sys.solutionUDotDot())
  {
    _need_dof_values_dotdot_neighbor = true;
    return _dof_values_dotdot_neighbor;
  }
  else
    mooseError("MooseVariableFE: Second time derivative of solution (`u_dotdot`) is not stored. "
               "Please set uDotDotRequested() to true in FEProblemBase before requesting "
               "`u_dotdot`.");
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotOldNeighbor()
{
  if (_sys.solutionUDotOld())
  {
    _need_dof_values_dot_old_neighbor = true;
    return _dof_values_dot_old_neighbor;
  }
  else
    mooseError("MooseVariableFE: Old time derivative of solution (`u_dot_old`) is not stored. "
               "Please set uDotOldRequested() to true in FEProblemBase before requesting "
               "`u_dot_old`.");
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotDotOldNeighbor()
{
  if (_sys.solutionUDotDotOld())
  {
    _need_dof_values_dotdot_old_neighbor = true;
    return _dof_values_dotdot_old_neighbor;
  }
  else
    mooseError("MooseVariableFE: Old second time derivative of solution (`u_dotdot_old`) is not "
               "stored. Please set uDotDotOldRequested() to true in FEProblemBase before "
               "requesting `u_dotdot_old`.");
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDuDotDu()
{
  _need_dof_du_dot_du = true;
  return _dof_du_dot_du;
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDuDotDotDu()
{
  _need_dof_du_dotdot_du = true;
  return _dof_du_dotdot_du;
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDuDotDuNeighbor()
{
  _need_dof_du_dot_du_neighbor = true;
  return _dof_du_dot_du_neighbor;
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDuDotDotDuNeighbor()
{
  _need_dof_du_dotdot_du_neighbor = true;
  return _dof_du_dotdot_du_neighbor;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::prepareIC()
{
  _dof_map.dof_indices(_elem, _dof_indices, _var_num);
  _dof_values.resize(_dof_indices.size());

  unsigned int nqp = _qrule->n_points();
  _u.resize(nqp);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeElemValues()
{
  computeValuesHelper(_qrule, _phi, _grad_phi, _second_phi, _curl_phi, _ad_grad_phi);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeElemValuesFace()
{
  computeValuesHelper(
      _qrule_face, _phi_face, _grad_phi_face, _second_phi_face, _curl_phi_face, _ad_grad_phi_face);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeNeighborValuesFace()
{
  computeNeighborValuesHelper(
      _qrule_neighbor, _phi_face_neighbor, _grad_phi_face_neighbor, _second_phi_face_neighbor);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeNeighborValues()
{
  computeNeighborValuesHelper(
      _qrule_neighbor, _phi_neighbor, _grad_phi_neighbor, _second_phi_neighbor);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeIncrementAtQps(const NumericVector<Number> & increment_vec)
{
  unsigned int nqp = _qrule->n_points();

  _increment.resize(nqp);
  // Compute the increment at each quadrature point
  unsigned int num_dofs = _dof_indices.size();
  for (unsigned int qp = 0; qp < nqp; qp++)
  {
    _increment[qp] = 0;
    for (unsigned int i = 0; i < num_dofs; i++)
      _increment[qp] += _phi[i][qp] * increment_vec(_dof_indices[i]);
  }
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeIncrementAtNode(const NumericVector<Number> & increment_vec)
{
  if (!isNodal())
    mooseError("computeIncrementAtNode can only be called for nodal variables");

  _increment.resize(1);

  // Compute the increment for the current DOF
  _increment[0] = increment_vec(_dof_indices[0]);
}

template <typename OutputType>
OutputType
MooseVariableFE<OutputType>::getValue(const Elem * elem,
                                      const std::vector<std::vector<OutputType>> & phi) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  OutputType value = 0;
  if (isNodal())
  {
    for (unsigned int i = 0; i < dof_indices.size(); ++i)
    {
      // The zero index is because we only have one point that the phis are evaluated at
      value += phi[i][0] * (*_sys.currentSolution())(dof_indices[i]);
    }
  }
  else
  {
    mooseAssert(dof_indices.size() == 1, "Wrong size for dof indices");
    value = (*_sys.currentSolution())(dof_indices[0]);
  }

  return value;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeValuesHelper(
    QBase *& qrule,
    const FieldVariablePhiValue & phi,
    const FieldVariablePhiGradient & grad_phi,
    const FieldVariablePhiSecond *& second_phi,
    const FieldVariablePhiValue *& curl_phi,
    const typename VariableTestGradientType<OutputType, JACOBIAN>::type & ad_grad_phi)

{
  unsigned int num_dofs = _dof_indices.size();

  if (num_dofs > 0)
    fetchDoFValues();

  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = qrule->n_points();
  auto & active_coupleable_vector_tags =
      _sys.subproblem().getActiveFEVariableCoupleableVectorTags(_tid);
  auto & active_coupleable_matrix_tags =
      _sys.subproblem().getActiveFEVariableCoupleableMatrixTags(_tid);

  _u.resize(nqp);
  _grad_u.resize(nqp);

  for (auto tag : active_coupleable_vector_tags)
    if (_need_vector_tag_u[tag])
      _vector_tag_u[tag].resize(nqp);

  for (auto tag : active_coupleable_matrix_tags)
    if (_need_matrix_tag_u[tag])
      _matrix_tag_u[tag].resize(nqp);

  if (_need_second)
    _second_u.resize(nqp);

  if (_need_curl)
    _curl_u.resize(nqp);

  if (_need_u_previous_nl)
    _u_previous_nl.resize(nqp);

  if (_need_grad_previous_nl)
    _grad_u_previous_nl.resize(nqp);

  if (_need_second_previous_nl)
    _second_u_previous_nl.resize(nqp);

  if (is_transient)
  {
    if (_need_u_dot)
      _u_dot.resize(nqp);

    if (_need_u_dotdot)
      _u_dotdot.resize(nqp);

    if (_need_u_dot_old)
      _u_dot_old.resize(nqp);

    if (_need_u_dotdot_old)
      _u_dotdot_old.resize(nqp);

    if (_need_du_dot_du)
      _du_dot_du.resize(nqp);

    if (_need_du_dotdot_du)
      _du_dotdot_du.resize(nqp);

    if (_need_grad_dot)
      _grad_u_dot.resize(nqp);

    if (_need_grad_dotdot)
      _grad_u_dotdot.resize(nqp);

    if (_need_u_old)
      _u_old.resize(nqp);

    if (_need_u_older)
      _u_older.resize(nqp);

    if (_need_grad_old)
      _grad_u_old.resize(nqp);

    if (_need_grad_older)
      _grad_u_older.resize(nqp);

    if (_need_second_old)
      _second_u_old.resize(nqp);

    if (_need_curl_old)
      _curl_u_old.resize(nqp);

    if (_need_second_older)
      _second_u_older.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u[i] = 0;
    _grad_u[i] = 0;

    for (auto tag : active_coupleable_vector_tags)
      if (_need_vector_tag_u[tag])
        _vector_tag_u[tag][i] = 0;

    for (auto tag : active_coupleable_matrix_tags)
      if (_need_matrix_tag_u[tag])
        _matrix_tag_u[tag][i] = 0;

    if (_need_second)
      _second_u[i] = 0;

    if (_need_curl)
      _curl_u[i] = 0;

    if (_need_u_previous_nl)
      _u_previous_nl[i] = 0;

    if (_need_grad_previous_nl)
      _grad_u_previous_nl[i] = 0;

    if (_need_second_previous_nl)
      _second_u_previous_nl[i] = 0;

    if (is_transient)
    {
      if (_need_u_dot)
        _u_dot[i] = 0;

      if (_need_u_dotdot)
        _u_dotdot[i] = 0;

      if (_need_u_dot_old)
        _u_dot_old[i] = 0;

      if (_need_u_dotdot_old)
        _u_dotdot_old[i] = 0;

      if (_need_du_dot_du)
        _du_dot_du[i] = 0;

      if (_need_du_dotdot_du)
        _du_dotdot_du[i] = 0;

      if (_need_grad_dot)
        _grad_u_dot[i] = 0;

      if (_need_grad_dotdot)
        _grad_u_dotdot[i] = 0;

      if (_need_u_old)
        _u_old[i] = 0;

      if (_need_u_older)
        _u_older[i] = 0;

      if (_need_grad_old)
        _grad_u_old[i] = 0;

      if (_need_grad_older)
        _grad_u_older[i] = 0;

      if (_need_second_old)
        _second_u_old[i] = 0;

      if (_need_second_older)
        _second_u_older[i] = 0;

      if (_need_curl_old)
        _curl_u_old[i] = 0;
    }
  }

  bool second_required =
      _need_second || _need_second_old || _need_second_older || _need_second_previous_nl;
  bool curl_required = _need_curl || _need_curl_old;

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    for (unsigned int qp = 0; qp < nqp; qp++)
    {
      const OutputType phi_local = phi[i][qp];
      const typename OutputTools<OutputType>::OutputGradient dphi_qp = grad_phi[i][qp];

      _u[qp] += phi_local * _dof_values[i];

      _grad_u[qp].add_scaled(dphi_qp, _dof_values[i]);

      if (is_transient)
      {
        if (_need_u_old)
          _u_old[qp] += phi_local * _dof_values_old[i];

        if (_need_u_older)
          _u_older[qp] += phi_local * _dof_values_older[i];

        if (_need_grad_old)
          _grad_u_old[qp].add_scaled(dphi_qp, _dof_values_old[i]);

        if (_need_grad_older)
          _grad_u_older[qp].add_scaled(dphi_qp, _dof_values_older[i]);

        if (_need_u_dot)
          _u_dot[qp] += phi_local * _dof_values_dot[i];

        if (_need_u_dotdot)
          _u_dotdot[qp] += phi_local * _dof_values_dotdot[i];

        if (_need_u_dot_old)
          _u_dot_old[qp] += phi_local * _dof_values_dot_old[i];

        if (_need_u_dotdot_old)
          _u_dotdot_old[qp] += phi_local * _dof_values_dotdot_old[i];

        if (_need_grad_dot)
          _grad_u_dot[qp].add_scaled(dphi_qp, _dof_values_dot[i]);

        if (_need_grad_dotdot)
          _grad_u_dotdot[qp].add_scaled(dphi_qp, _dof_values_dotdot[i]);

        if (_need_du_dot_du)
          _du_dot_du[qp] = _dof_du_dot_du[i];

        if (_need_du_dotdot_du)
          _du_dotdot_du[qp] = _dof_du_dotdot_du[i];
      }

      if (second_required)
      {
        libmesh_assert(second_phi);
        const typename OutputTools<OutputType>::OutputSecond d2phi_local = (*second_phi)[i][qp];

        if (_need_second)
          _second_u[qp].add_scaled(d2phi_local, _dof_values[i]);

        if (_need_second_previous_nl)
          _second_u_previous_nl[qp].add_scaled(d2phi_local, _dof_values_previous_nl[i]);

        if (is_transient)
        {
          if (_need_second_old)
            _second_u_old[qp].add_scaled(d2phi_local, _dof_values_old[i]);

          if (_need_second_older)
            _second_u_older[qp].add_scaled(d2phi_local, _dof_values_older[i]);
        }
      }

      if (curl_required)
      {
        libmesh_assert(curl_phi);
        const OutputType curl_phi_local = (*curl_phi)[i][qp];

        if (_need_curl)
          _curl_u[qp] += curl_phi_local * _dof_values[i];

        if (is_transient && _need_curl_old)
          _curl_u_old[qp] += curl_phi_local * _dof_values_old[i];
      }

      for (auto tag : active_coupleable_vector_tags)
        if (_need_vector_tag_u[tag])
          _vector_tag_u[tag][qp] += phi_local * _vector_tags_dof_u[tag][i];

      for (auto tag : active_coupleable_matrix_tags)
        if (_need_matrix_tag_u[tag])
          _matrix_tag_u[tag][qp] += phi_local * _matrix_tags_dof_u[tag][i];

      if (_need_u_previous_nl)
        _u_previous_nl[qp] += phi_local * _dof_values_previous_nl[i];

      if (_need_grad_previous_nl)
        _grad_u_previous_nl[qp].add_scaled(dphi_qp, _dof_values_previous_nl[i]);
    }
  }

  // Automatic differentiation
  if (_need_ad && _subproblem.currentlyComputingJacobian())
    computeAD(num_dofs, nqp, is_transient, phi, grad_phi, second_phi, ad_grad_phi);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeAD(
    const unsigned int num_dofs,
    const unsigned int nqp,
    const bool /*is_transient*/,
    const FieldVariablePhiValue & phi,
    const FieldVariablePhiGradient & grad_phi,
    const FieldVariablePhiSecond *& second_phi,
    const typename VariableTestGradientType<OutputType, JACOBIAN>::type & ad_grad_phi)
{
  _ad_dof_values.resize(num_dofs);
  if (_need_ad_u)
    _ad_u.resize(nqp);

  if (_need_ad_grad_u)
    _ad_grad_u.resize(nqp);

  if (_need_ad_second_u)
    _ad_second_u.resize(nqp);

  if (_need_ad_u_dot)
  {
    _ad_dofs_dot.resize(num_dofs);
    _ad_u_dot.resize(nqp);
  }

  // Derivatives are offset by the variable number
  size_t ad_offset = _var_num * _sys.getMaxVarNDofsPerElem();

  // Hopefully this problem can go away at some point
  if (ad_offset + num_dofs > AD_MAX_DOFS_PER_ELEM)
    mooseError("Current number of dofs per element is greater than AD_MAX_DOFS_PER_ELEM of ",
               AD_MAX_DOFS_PER_ELEM);

  for (unsigned int qp = 0; qp < nqp; qp++)
  {
    if (_need_ad_u)
      _ad_u[qp] = _ad_zero;

    if (_need_ad_grad_u)
      _ad_grad_u[qp] = _ad_zero;

    if (_need_ad_second_u)
      _ad_second_u[qp] = _ad_zero;

    if (_need_ad_u_dot)
      _ad_u_dot[qp] = _ad_zero;
  }

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    _ad_dof_values[i] = (*_sys.currentSolution())(_dof_indices[i]);

    // NOTE!  You have to do this AFTER setting the value!
    if (_var_kind == Moose::VAR_NONLINEAR)
      _ad_dof_values[i].derivatives()[ad_offset + i] = 1.0;

    if (_need_ad_u_dot && _time_integrator)
    {
      _ad_dofs_dot[i] = _ad_dof_values[i];
      _time_integrator->computeADTimeDerivatives(_ad_dofs_dot[i], _dof_indices[i]);
    }
  }

  // Now build up the solution at each quadrature point:
  for (unsigned int i = 0; i < num_dofs; i++)
  {
    for (unsigned int qp = 0; qp < nqp; qp++)
    {
      if (_need_ad_u)
        _ad_u[qp] += _ad_dof_values[i] * phi[i][qp];

      if (_need_ad_grad_u)
      {
        if (_displaced)
          _ad_grad_u[qp] += _ad_dof_values[i] * ad_grad_phi[i][qp];
        else
          _ad_grad_u[qp] += _ad_dof_values[i] * grad_phi[i][qp];
      }

      if (_need_ad_second_u)
      {
        if (_displaced)
          mooseError("Support for second order shape function derivatives on the displaced mesh "
                     "has not been added yet!");
        else
          _ad_second_u[qp] += _ad_dof_values[i] * (*second_phi)[i][qp];
      }

      if (_need_ad_u_dot && _time_integrator)
        _ad_u_dot[qp] += phi[i][qp] * _ad_dofs_dot[i];
    }
  }

  if (_need_ad_u_dot && !_time_integrator)
    for (MooseIndex(nqp) qp = 0; qp < nqp; ++qp)
      _ad_u_dot[qp] = _u_dot[qp];
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeADNeighbor(const unsigned int num_dofs,
                                               const unsigned int nqp,
                                               const bool /*is_transient*/,
                                               const FieldVariablePhiValue & phi,
                                               const FieldVariablePhiGradient & grad_phi,
                                               const FieldVariablePhiSecond *& second_phi)

{
  _neighbor_ad_dof_values.resize(num_dofs);
  if (_need_neighbor_ad_u)
    _neighbor_ad_u.resize(nqp);

  if (_need_neighbor_ad_grad_u)
    _neighbor_ad_grad_u.resize(nqp);

  if (_need_neighbor_ad_second_u)
    _neighbor_ad_second_u.resize(nqp);

  if (_need_neighbor_ad_u_dot)
    _neighbor_ad_u_dot.resize(nqp);

  // Derivatives are offset by the variable number
  size_t ad_offset = _var_num * _sys.getMaxVarNDofsPerElem() +
                     (_sys.system().n_vars() * _sys.getMaxVarNDofsPerElem());

  // Hopefully this problem can go away at some point
  if (ad_offset + num_dofs > AD_MAX_DOFS_PER_ELEM)
    mooseError("Current number of dofs per element is greater than AD_MAX_DOFS_PER_ELEM of ",
               AD_MAX_DOFS_PER_ELEM);

  for (unsigned int qp = 0; qp < nqp; qp++)
  {
    if (_need_neighbor_ad_u)
      _neighbor_ad_u[qp] = _ad_zero;

    if (_need_neighbor_ad_grad_u)
      _neighbor_ad_grad_u[qp] = _ad_zero;

    if (_need_neighbor_ad_second_u)
      _neighbor_ad_second_u[qp] = _ad_zero;

    if (_need_neighbor_ad_u_dot)
      _neighbor_ad_u_dot[qp] = _ad_zero;
  }

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    _neighbor_ad_dof_values[i] = (*_sys.currentSolution())(_dof_indices_neighbor[i]);

    // NOTE!  You have to do this AFTER setting the value!
    if (_var_kind == Moose::VAR_NONLINEAR)
      _neighbor_ad_dof_values[i].derivatives()[ad_offset + i] = 1.0;

    if (_need_neighbor_ad_u_dot && _time_integrator)
    {
      _neighbor_ad_dofs_dot[i] = _neighbor_ad_dof_values[i];
      _time_integrator->computeADTimeDerivatives(_neighbor_ad_dofs_dot[i],
                                                 _dof_indices_neighbor[i]);
    }
  }

  // Now build up the solution at each quadrature point:
  for (unsigned int i = 0; i < num_dofs; i++)
  {
    for (unsigned int qp = 0; qp < nqp; qp++)
    {
      if (_need_neighbor_ad_u)
        _neighbor_ad_u[qp] += _neighbor_ad_dof_values[i] * phi[i][qp];

      if (_need_neighbor_ad_grad_u)
        _neighbor_ad_grad_u[qp] += _neighbor_ad_dof_values[i] * grad_phi[i][qp];

      if (_need_neighbor_ad_second_u)
        _neighbor_ad_second_u[qp] += _neighbor_ad_dof_values[i] * (*second_phi)[i][qp];

      if (_need_neighbor_ad_u_dot && _time_integrator)
        _neighbor_ad_u_dot[qp] += phi[i][qp] * _neighbor_ad_dofs_dot[i];
    }
  }

  if (_need_neighbor_ad_u_dot && !_time_integrator)
    for (MooseIndex(nqp) qp = 0; qp < nqp; ++qp)
      _neighbor_ad_u_dot[qp] = _u_dot_neighbor[qp];
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeNeighborValuesHelper(QBase *& qrule,
                                                         const FieldVariablePhiValue & phi,
                                                         const FieldVariablePhiGradient & grad_phi,
                                                         const FieldVariablePhiSecond *& second_phi)
{
  unsigned int num_dofs = _dof_indices_neighbor.size();

  if (num_dofs > 0)
    fetchDoFValuesNeighbor();

  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = qrule->n_points();

  _u_neighbor.resize(nqp);
  _grad_u_neighbor.resize(nqp);

  if (_need_second_neighbor)
    _second_u_neighbor.resize(nqp);

  if (is_transient)
  {
    if (_need_u_dot_neighbor)
      _u_dot_neighbor.resize(nqp);

    if (_need_u_dotdot_neighbor)
      _u_dotdot_neighbor.resize(nqp);

    if (_need_u_dot_old_neighbor)
      _u_dot_old_neighbor.resize(nqp);

    if (_need_u_dotdot_old_neighbor)
      _u_dotdot_old_neighbor.resize(nqp);

    if (_need_du_dot_du_neighbor)
      _du_dot_du_neighbor.resize(nqp);

    if (_need_du_dotdot_du_neighbor)
      _du_dotdot_du_neighbor.resize(nqp);

    if (_need_grad_neighbor_dot)
      _grad_u_neighbor_dot.resize(nqp);

    if (_need_grad_neighbor_dotdot)
      _grad_u_neighbor_dotdot.resize(nqp);

    if (_need_u_old_neighbor)
      _u_old_neighbor.resize(nqp);

    if (_need_u_older_neighbor)
      _u_older_neighbor.resize(nqp);

    if (_need_grad_old_neighbor)
      _grad_u_old_neighbor.resize(nqp);

    if (_need_grad_older_neighbor)
      _grad_u_older_neighbor.resize(nqp);

    if (_need_second_old_neighbor)
      _second_u_old_neighbor.resize(nqp);

    if (_need_second_older_neighbor)
      _second_u_older_neighbor.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u_neighbor[i] = 0;
    _grad_u_neighbor[i] = 0;

    if (_need_second_neighbor)
      _second_u_neighbor[i] = 0;

    if (_need_grad_dot)
      _grad_u_neighbor_dot[i] = 0;
    if (_need_grad_dotdot)
      _grad_u_neighbor_dotdot[i] = 0;

    if (is_transient)
    {
      if (_need_u_dot_neighbor)
        _u_dot_neighbor[i] = 0;

      if (_need_u_dotdot_neighbor)
        _u_dotdot_neighbor[i] = 0;

      if (_need_u_dot_old_neighbor)
        _u_dot_old_neighbor[i] = 0;

      if (_need_u_dotdot_old_neighbor)
        _u_dotdot_old_neighbor[i] = 0;

      if (_need_du_dot_du_neighbor)
        _du_dot_du_neighbor[i] = 0;

      if (_need_du_dotdot_du_neighbor)
        _du_dotdot_du_neighbor[i] = 0;

      if (_need_u_old_neighbor)
        _u_old_neighbor[i] = 0;

      if (_need_u_older_neighbor)
        _u_older_neighbor[i] = 0;

      if (_need_grad_old_neighbor)
        _grad_u_old_neighbor[i] = 0;

      if (_need_grad_older_neighbor)
        _grad_u_older_neighbor[i] = 0;

      if (_need_second_old_neighbor)
        _second_u_old_neighbor[i] = 0;

      if (_need_second_older_neighbor)
        _second_u_older_neighbor[i] = 0;
    }
  }

  bool second_required =
      _need_second_neighbor || _need_second_old_neighbor || _need_second_older_neighbor;

  for (unsigned int i = 0; i < num_dofs; ++i)
  {
    for (unsigned int qp = 0; qp < nqp; ++qp)
    {
      OutputType phi_local = phi[i][qp];
      typename OutputTools<OutputType>::OutputGradient dphi_local = grad_phi[i][qp];

      _u_neighbor[qp] += phi_local * _dof_values_neighbor[i];

      _grad_u_neighbor[qp] += dphi_local * _dof_values_neighbor[i];

      if (is_transient)
      {
        if (_need_u_dot_neighbor)
          _u_dot_neighbor[qp] += phi_local * _dof_values_dot_neighbor[i];

        if (_need_u_dotdot_neighbor)
          _u_dotdot_neighbor[qp] += phi_local * _dof_values_dotdot_neighbor[i];

        if (_need_u_dot_old_neighbor)
          _u_dot_old_neighbor[qp] += phi_local * _dof_values_dot_old_neighbor[i];

        if (_need_u_dotdot_old_neighbor)
          _u_dotdot_old_neighbor[qp] += phi_local * _dof_values_dotdot_old_neighbor[i];

        if (_need_du_dot_du_neighbor)
          _du_dot_du_neighbor[qp] = _dof_du_dot_du_neighbor[i];

        if (_need_du_dotdot_du_neighbor)
          _du_dotdot_du_neighbor[qp] = _dof_du_dotdot_du_neighbor[i];

        if (_need_grad_neighbor_dot)
          _grad_u_neighbor_dot[qp].add_scaled(dphi_local, _dof_values_dot_neighbor[i]);

        if (_need_grad_neighbor_dotdot)
          _grad_u_neighbor_dotdot[qp].add_scaled(dphi_local, _dof_values_dotdot_neighbor[i]);

        if (_need_u_old_neighbor)
          _u_old_neighbor[qp] += phi_local * _dof_values_old_neighbor[i];

        if (_need_u_older_neighbor)
          _u_older_neighbor[qp] += phi_local * _dof_values_older_neighbor[i];

        if (_need_grad_old_neighbor)
          _grad_u_old_neighbor[qp] += dphi_local * _dof_values_old_neighbor[i];

        if (_need_grad_older_neighbor)
          _grad_u_older_neighbor[qp] += dphi_local * _dof_values_older_neighbor[i];
      }

      if (second_required)
      {
        libmesh_assert(second_phi);
        typename OutputTools<OutputType>::OutputSecond d2phi_local = (*second_phi)[i][qp];

        if (_need_second_neighbor)
          _second_u_neighbor[qp] += d2phi_local * _dof_values_neighbor[i];

        if (is_transient)
        {
          if (_need_second_old_neighbor)
            _second_u_old_neighbor[qp] += d2phi_local * _dof_values_old_neighbor[i];

          if (_need_second_older_neighbor)
            _second_u_older_neighbor[qp] += d2phi_local * _dof_values_older_neighbor[i];
        }
      }
    }
  }

  // Automatic differentiation
  if (_need_neighbor_ad && _subproblem.currentlyComputingJacobian())
    computeADNeighbor(num_dofs, nqp, is_transient, phi, grad_phi, second_phi);
}

template <typename OutputType>
typename OutputTools<OutputType>::OutputGradient
MooseVariableFE<OutputType>::getGradient(
    const Elem * elem,
    const std::vector<std::vector<typename OutputTools<OutputType>::OutputGradient>> & grad_phi)
    const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  typename OutputTools<OutputType>::OutputGradient value;
  if (isNodal())
  {
    for (unsigned int i = 0; i < dof_indices.size(); ++i)
    {
      // The zero index is because we only have one point that the phis are evaluated at
      value += grad_phi[i][0] * (*_sys.currentSolution())(dof_indices[i]);
    }
  }
  else
  {
    mooseAssert(dof_indices.size() == 1, "Wrong size for dof indices");
    value = 0.0;
  }

  return value;
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValue()
{
  if (isNodal())
  {
    _need_dof_values = true;
    return _nodal_value;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueNeighbor()
{
  if (isNodal())
  {
    _need_dof_values_neighbor = true;
    return _neighbor_nodal_value;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

template <typename OutputType>
const MooseArray<Real> &
MooseVariableFE<OutputType>::nodalVectorTagValue(TagID tag)
{
  if (isNodal())
  {
    _need_vector_tag_dof_u[tag] = true;

    if (_sys.hasVector(tag) && tag < _vector_tags_dof_u.size())
      return _vector_tags_dof_u[tag];
    else
      mooseError("Tag is not associated with any vector or there is no any data for tag ",
                 tag,
                 " for nodal variable ",
                 name());
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

template <typename OutputType>
const MooseArray<Real> &
MooseVariableFE<OutputType>::nodalMatrixTagValue(TagID tag)
{
  if (isNodal())
  {
    _need_matrix_tag_dof_u[tag] = true;

    if (_sys.hasMatrix(tag) && tag < _matrix_tags_dof_u.size())
      return _matrix_tags_dof_u[tag];
    else
      mooseError("Tag is not associated with any matrix or there is no any data for tag ",
                 tag,
                 " for nodal variable ",
                 name());
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueOld()
{
  if (isNodal())
  {
    _need_dof_values_old = true;
    return _nodal_value_old;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueOldNeighbor()
{
  if (isNodal())
  {
    _need_dof_values_old_neighbor = true;
    return _neighbor_nodal_value_old;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueOlder()
{
  if (isNodal())
  {
    _need_dof_values_older = true;
    return _nodal_value_older;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueOlderNeighbor()
{
  if (isNodal())
  {
    _need_dof_values_older_neighbor = true;
    return _neighbor_nodal_value_older;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValuePreviousNL()
{
  if (isNodal())
  {
    _need_dof_values_previous_nl = true;
    return _nodal_value_previous_nl;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValuePreviousNLNeighbor()
{
  if (isNodal())
  {
    _need_dof_values_previous_nl_neighbor = true;
    return _neighbor_nodal_value_previous_nl;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueDot()
{
  if (isNodal())
  {
    if (_sys.solutionUDot())
    {
      _need_dof_values_dot = true;
      return _nodal_value_dot;
    }
    else
      mooseError("MooseVariableFE: Time derivative of solution (`u_dot`) is not stored. Please set "
                 "uDotRequested() to true in FEProblemBase before requesting `u_dot`.");
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueDotDot()
{
  if (isNodal())
  {
    if (_sys.solutionUDotDot())
    {
      _need_dof_values_dotdot = true;
      return _nodal_value_dotdot;
    }
    else
      mooseError("MooseVariableFE: Second time derivative of solution (`u_dotdot`) is not stored. "
                 "Please set uDotDotRequested() to true in FEProblemBase before requesting "
                 "`u_dotdot`.");
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueDotOld()
{
  if (isNodal())
  {
    if (_sys.solutionUDotOld())
    {
      _need_dof_values_dot_old = true;
      return _nodal_value_dot_old;
    }
    else
      mooseError("MooseVariableFE: Old time derivative of solution (`u_dot_old`) is not stored. "
                 "Please set uDotOldRequested() to true in FEProblemBase before requesting "
                 "`u_dot_old`.");
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueDotDotOld()
{
  if (isNodal())
  {
    if (_sys.solutionUDotDotOld())
    {
      _need_dof_values_dotdot_old = true;
      return _nodal_value_dotdot_old;
    }
    else
      mooseError("MooseVariableFE: Old second time derivative of solution (`u_dotdot_old`) is not "
                 "stored. Please set uDotDotOldRequested() to true in FEProblemBase before "
                 "requesting `u_dotdot_old`.");
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeNodalValues()
{
  if (_has_dof_indices)
  {
    fetchDoFValues();
    assignNodalValue();

    if (_need_ad && _subproblem.currentlyComputingJacobian())
      fetchADDoFValues();
  }
  else
    zeroSizeDofValues();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeNodalNeighborValues()
{
  if (_neighbor_has_dof_indices)
  {
    fetchDoFValuesNeighbor();
    assignNodalValueNeighbor();
  }
  else
    zeroSizeDofValuesNeighbor();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::fetchDoFValues()
{
  bool is_transient = _subproblem.isTransient();

  auto n = _dof_indices.size();
  libmesh_assert(n);

  _dof_values.resize(n);
  _sys.currentSolution()->get(_dof_indices, &_dof_values[0]);

  if (is_transient)
  {
    if (_need_u_old || _need_grad_old || _need_second_old || _need_curl_old || _need_dof_values_old)
    {
      _dof_values_old.resize(n);
      _sys.solutionOld().get(_dof_indices, &_dof_values_old[0]);
    }
    if (_need_u_older || _need_grad_older || _need_second_older || _need_dof_values_older)
    {
      _dof_values_older.resize(n);
      _sys.solutionOlder().get(_dof_indices, &_dof_values_older[0]);
    }
    if (_need_u_dot || _need_grad_dot || _need_dof_values_dot)
    {
      libmesh_assert(_sys.solutionUDot());
      _dof_values_dot.resize(n);
      _sys.solutionUDot()->get(_dof_indices, &_dof_values_dot[0]);
    }
    if (_need_u_dotdot || _need_grad_dotdot || _need_dof_values_dotdot)
    {
      libmesh_assert(_sys.solutionUDotDot());
      _dof_values_dotdot.resize(n);
      _sys.solutionUDotDot()->get(_dof_indices, &_dof_values_dotdot[0]);
    }
    if (_need_u_dot_old || _need_dof_values_dot_old)
    {
      libmesh_assert(_sys.solutionUDotOld());
      _dof_values_dot_old.resize(n);
      _sys.solutionUDotOld()->get(_dof_indices, &_dof_values_dot_old[0]);
    }
    if (_need_u_dotdot_old || _need_dof_values_dotdot_old)
    {
      libmesh_assert(_sys.solutionUDotDotOld());
      _dof_values_dotdot_old.resize(n);
      _sys.solutionUDotDotOld()->get(_dof_indices, &_dof_values_dotdot_old[0]);
    }
  }

  if (_need_u_previous_nl || _need_grad_previous_nl || _need_second_previous_nl ||
      _need_dof_values_previous_nl)
  {
    _dof_values_previous_nl.resize(n);
    _sys.solutionPreviousNewton()->get(_dof_indices, &_dof_values_previous_nl[0]);
  }

  if (_sys.subproblem().safeAccessTaggedVectors())
  {
    auto & active_coupleable_vector_tags =
        _sys.subproblem().getActiveFEVariableCoupleableVectorTags(_tid);
    for (auto tag : active_coupleable_vector_tags)
      if (_need_vector_tag_u[tag] || _need_vector_tag_dof_u[tag])
        if (_sys.hasVector(tag) && _sys.getVector(tag).closed())
        {
          auto & vec = _sys.getVector(tag);
          _vector_tags_dof_u[tag].resize(n);
          vec.get(_dof_indices, &_vector_tags_dof_u[tag][0]);
        }
  }

  if (_sys.subproblem().safeAccessTaggedMatrices())
  {
    auto & active_coupleable_matrix_tags =
        _sys.subproblem().getActiveFEVariableCoupleableMatrixTags(_tid);
    for (auto tag : active_coupleable_matrix_tags)
    {
      _matrix_tags_dof_u[tag].resize(n);
      if (_need_matrix_tag_dof_u[tag] || _need_matrix_tag_u[tag])
        if (_sys.hasMatrix(tag) && _sys.matrixTagActive(tag) && _sys.getMatrix(tag).closed())
        {
          auto & mat = _sys.getMatrix(tag);
          for (unsigned i = 0; i < _dof_indices.size(); i++)
          {
            Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
            _matrix_tags_dof_u[tag][i] = mat(_dof_indices[i], _dof_indices[i]);
          }
        }
    }
  }

  if (_need_du_dot_du || _need_dof_du_dot_du)
  {
    _dof_du_dot_du.resize(n);
    for (decltype(n) i = 0; i < n; ++i)
      _dof_du_dot_du[i] = _sys.duDotDu();
  }
  if (_need_du_dotdot_du || _need_dof_du_dotdot_du)
  {
    _dof_du_dotdot_du.resize(n);
    for (decltype(n) i = 0; i < n; ++i)
      _dof_du_dotdot_du[i] = _sys.duDotDotDu();
  }
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::fetchADDoFValues()
{
  auto n = _dof_indices.size();
  libmesh_assert(n);
  _ad_dof_values.resize(n);
  auto ad_offset = _var_num * _sys.getMaxVarNDofsPerNode();

  for (decltype(n) i = 0; i < n; ++i)
  {
    _ad_dof_values[i] = _dof_values[i];
    if (_var_kind == Moose::VAR_NONLINEAR)
      _ad_dof_values[i].derivatives()[ad_offset + i] = 1.;
    assignADNodalValue(_ad_dof_values[i], i);
  }
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::zeroSizeDofValues()
{
  _dof_values.resize(0);
  if (_need_dof_values_previous_nl)
    _dof_values_previous_nl.resize(0);
  if (_subproblem.isTransient())
  {
    _dof_values_old.resize(0);
    _dof_values_older.resize(0);
    _dof_values_dot.resize(0);
    _dof_values_dotdot.resize(0);
    _dof_values_dot_old.resize(0);
    _dof_values_dotdot_old.resize(0);
    _dof_du_dot_du.resize(0);
    _dof_du_dotdot_du.resize(0);
  }
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::assignNodalValue()
{
  bool is_transient = _subproblem.isTransient();

  libmesh_assert(_dof_indices.size());

  _nodal_value = _dof_values[0];

  if (is_transient)
  {
    if (_need_dof_values_old)
      _nodal_value_old = _dof_values_old[0];
    if (_need_dof_values_older)
      _nodal_value_older = _dof_values_older[0];
    if (_need_dof_values_dot)
      _nodal_value_dot = _dof_values_dot[0];
    if (_need_dof_values_dotdot)
      _nodal_value_dotdot = _dof_values_dotdot[0];
    if (_need_dof_values_dot_old)
      _nodal_value_dot_old = _dof_values_dot_old[0];
    if (_need_dof_values_dotdot_old)
      _nodal_value_dotdot_old = _dof_values_dotdot_old[0];
  }
  if (_need_dof_values_previous_nl)
    _nodal_value_previous_nl = _dof_values_previous_nl[0];
}

template <>
void
MooseVariableFE<RealVectorValue>::assignNodalValue()
{
  bool is_transient = _subproblem.isTransient();

  auto n = _dof_indices.size();
  libmesh_assert(n);

  for (decltype(n) i = 0; i < n; ++i)
    _nodal_value(i) = _dof_values[i];

  if (is_transient)
  {
    if (_need_dof_values_old)
      for (decltype(n) i = 0; i < n; ++i)
        _nodal_value_old(i) = _dof_values_old[i];
    if (_need_dof_values_older)
      for (decltype(n) i = 0; i < n; ++i)
        _nodal_value_older(i) = _dof_values_older[i];
    if (_need_dof_values_dot)
      for (decltype(n) i = 0; i < n; ++i)
        _nodal_value_dot(i) = _dof_values_dot[i];
    if (_need_dof_values_dotdot)
      for (decltype(n) i = 0; i < n; ++i)
        _nodal_value_dotdot(i) = _dof_values_dotdot[i];
    if (_need_dof_values_dot_old)
      for (decltype(n) i = 0; i < n; ++i)
        _nodal_value_dot_old(i) = _dof_values_dot_old[i];
    if (_need_dof_values_dotdot_old)
      for (decltype(n) i = 0; i < n; ++i)
        _nodal_value_dotdot_old(i) = _dof_values_dotdot_old[i];
  }
  if (_need_dof_values_previous_nl)
    for (decltype(n) i = 0; i < n; ++i)
      _nodal_value_previous_nl(i) = _dof_values_previous_nl[i];
}

template <>
void
MooseVariableFE<Real>::assignADNodalValue(const DualReal & value, const unsigned int &)
{
  _ad_nodal_value = value;
}

template <>
void
MooseVariableFE<RealVectorValue>::assignADNodalValue(const DualReal & value,
                                                     const unsigned int & component)
{
  _ad_nodal_value(component) = value;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::fetchDoFValuesNeighbor()
{
  bool is_transient = _subproblem.isTransient();

  auto n = _dof_indices_neighbor.size();
  libmesh_assert(n);

  _dof_values_neighbor.resize(n);
  _sys.currentSolution()->get(_dof_indices_neighbor, &_dof_values_neighbor[0]);

  if (_need_u_previous_nl_neighbor || _need_grad_previous_nl_neighbor ||
      _need_second_previous_nl_neighbor || _need_dof_values_previous_nl_neighbor)
  {
    _dof_values_previous_nl_neighbor.resize(n);
    _sys.solutionPreviousNewton()->get(_dof_indices_neighbor, &_dof_values_previous_nl_neighbor[0]);
  }

  if (is_transient)
  {
    if (_need_u_old_neighbor || _need_grad_old_neighbor || _need_second_old_neighbor ||
        _need_dof_values_old_neighbor)
    {
      _dof_values_old_neighbor.resize(n);
      _sys.solutionOld().get(_dof_indices_neighbor, &_dof_values_old_neighbor[0]);
    }
    if (_need_u_older_neighbor || _need_grad_older_neighbor || _need_second_older_neighbor ||
        _need_dof_values_older_neighbor)
    {
      _dof_values_older_neighbor.resize(n);
      _sys.solutionOlder().get(_dof_indices_neighbor, &_dof_values_older_neighbor[0]);
    }
    if (_need_u_dot_neighbor || _need_grad_neighbor_dot || _need_dof_values_dot_neighbor)
    {
      _dof_values_dot_neighbor.resize(n);
      libmesh_assert(_sys.solutionUDot());
      for (unsigned int i = 0; i < n; i++)
        _dof_values_dot_neighbor[i] = (*_sys.solutionUDot())(_dof_indices_neighbor[i]);
    }
    if (_need_u_dotdot_neighbor || _need_grad_neighbor_dotdot || _need_dof_values_dotdot_neighbor)
    {
      _dof_values_dotdot_neighbor.resize(n);
      libmesh_assert(_sys.solutionUDotDot());
      for (unsigned int i = 0; i < n; i++)
        _dof_values_dotdot_neighbor[i] = (*_sys.solutionUDotDot())(_dof_indices_neighbor[i]);
    }
    if (_need_u_dot_old_neighbor || _need_dof_values_dot_old_neighbor)
    {
      _dof_values_dot_old_neighbor.resize(n);
      libmesh_assert(_sys.solutionUDotOld());
      for (unsigned int i = 0; i < n; i++)
        _dof_values_dot_old_neighbor[i] = (*_sys.solutionUDotOld())(_dof_indices_neighbor[i]);
    }
    if (_need_u_dotdot_old_neighbor || _need_dof_values_dotdot_old_neighbor)
    {
      _dof_values_dotdot_old_neighbor.resize(n);
      libmesh_assert(_sys.solutionUDotDotOld());
      for (unsigned int i = 0; i < n; i++)
        _dof_values_dotdot_old_neighbor[i] = (*_sys.solutionUDotDotOld())(_dof_indices_neighbor[i]);
    }
  }
  if (_need_du_dot_du_neighbor || _need_dof_du_dot_du_neighbor)
  {
    _dof_du_dot_du_neighbor.resize(n);
    for (decltype(n) i = 0; i < n; ++i)
      _dof_du_dot_du_neighbor[i] = _sys.duDotDu();
  }
  if (_need_du_dotdot_du_neighbor || _need_dof_du_dotdot_du_neighbor)
  {
    _dof_du_dotdot_du_neighbor.resize(n);
    for (decltype(n) i = 0; i < n; ++i)
      _dof_du_dotdot_du_neighbor[i] = _sys.duDotDotDu();
  }
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::assignNodalValueNeighbor()
{
  bool is_transient = _subproblem.isTransient();

  libmesh_assert(_dof_indices_neighbor.size());

  _neighbor_nodal_value = _dof_values_neighbor[0];

  if (_need_dof_values_previous_nl_neighbor)
    _neighbor_nodal_value_previous_nl = _dof_values_previous_nl_neighbor[0];

  if (is_transient)
  {
    if (_need_dof_values_old_neighbor)
      _neighbor_nodal_value_old = _dof_values_old_neighbor[0];
    if (_need_dof_values_older_neighbor)
      _neighbor_nodal_value_older = _dof_values_older_neighbor[0];
  }
}

template <>
void
MooseVariableFE<RealVectorValue>::assignNodalValueNeighbor()
{
  bool is_transient = _subproblem.isTransient();

  auto n = _dof_indices_neighbor.size();
  libmesh_assert(n);

  for (unsigned int i = 0; i < n; ++i)
    _neighbor_nodal_value(i) = _dof_values_neighbor[i];

  if (_need_dof_values_previous_nl_neighbor)
    for (unsigned int i = 0; i < n; ++i)
      _neighbor_nodal_value_previous_nl(i) = _dof_values_previous_nl_neighbor[i];

  if (is_transient)
  {
    if (_need_dof_values_old_neighbor)
      for (unsigned int i = 0; i < n; ++i)
        _neighbor_nodal_value_old(i) = _dof_values_old_neighbor[i];
    if (_need_dof_values_older_neighbor)
      for (unsigned int i = 0; i < n; ++i)
        _neighbor_nodal_value_older(i) = _dof_values_older_neighbor[i];
  }
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::zeroSizeDofValuesNeighbor()
{
  _dof_values_neighbor.resize(0);
  if (_need_dof_values_previous_nl_neighbor)
    _dof_values_previous_nl_neighbor.resize(0);
  if (_subproblem.isTransient())
  {
    _dof_values_old_neighbor.resize(0);
    _dof_values_older_neighbor.resize(0);
    _dof_values_dot_neighbor.resize(0);
    _dof_values_dotdot_neighbor.resize(0);
    _dof_values_dot_old_neighbor.resize(0);
    _dof_values_dotdot_old_neighbor.resize(0);
    _dof_du_dot_du_neighbor.resize(0);
    _dof_du_dotdot_du_neighbor.resize(0);
  }
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::setDofValues(const DenseVector<Number> & values)
{
  for (unsigned int i = 0; i < values.size(); i++)
    _dof_values[i] = values(i);

  _has_dof_values = true;

  for (unsigned int qp = 0; qp < _u.size(); qp++)
  {
    _u[qp] = 0;
    for (unsigned int i = 0; i < _dof_values.size(); i++)
      _u[qp] += _phi[i][qp] * _dof_values[i];
  }
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::setNodalValue(OutputType value, unsigned int idx /* = 0*/)
{
  _dof_values[idx] = value; // update variable nodal value
  _has_dof_values = true;
  _nodal_value = value;

  // Update the qp values as well
  for (unsigned int qp = 0; qp < _u.size(); qp++)
    _u[qp] = value;
}

template <>
void
MooseVariableFE<RealVectorValue>::setNodalValue(RealVectorValue value, unsigned int idx /* = 0*/)
{
  for (decltype(idx) i = 0; i < LIBMESH_DIM; ++i, ++idx)
  {
    _dof_values[idx] = value(i);
    _nodal_value(i) = value(i);
  }

  // Update the qp values as well
  for (unsigned int qp = 0; qp < _u.size(); qp++)
    _u[qp] = value;
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::isVector() const
{
  return std::is_same<OutputType, RealVectorValue>::value;
}

template <>
template <>
const VariableValue &
MooseVariableFE<Real>::adSln<RESIDUAL>()
{
  return _u;
}

template <>
template <>
const VectorVariableValue &
MooseVariableFE<RealVectorValue>::adSln<RESIDUAL>()
{
  return _u;
}

template <>
template <>
const VariableGradient &
MooseVariableFE<Real>::adGradSln<RESIDUAL>()
{
  return _grad_u;
}

template <>
template <>
const VectorVariableGradient &
MooseVariableFE<RealVectorValue>::adGradSln<RESIDUAL>()
{
  return _grad_u;
}

template <>
template <>
const VariableSecond &
MooseVariableFE<Real>::adSecondSln<RESIDUAL>()
{
  _need_second = true;
  secondPhi();
  secondPhiFace();
  return _second_u;
}

template <>
template <>
const VectorVariableSecond &
MooseVariableFE<RealVectorValue>::adSecondSln<RESIDUAL>()
{
  _need_second = true;
  secondPhi();
  secondPhiFace();
  return _second_u;
}

template <>
template <>
const VariableValue &
MooseVariableFE<Real>::adUDot<RESIDUAL>()
{

  return uDot();
}

template <>
template <>
const VectorVariableValue &
MooseVariableFE<RealVectorValue>::adUDot<RESIDUAL>()
{
  return uDot();
}

template <>
template <>
const VariableValue &
MooseVariableFE<Real>::adSlnNeighbor<RESIDUAL>()
{
  return _u_neighbor;
}

template <>
template <>
const VectorVariableValue &
MooseVariableFE<RealVectorValue>::adSlnNeighbor<RESIDUAL>()
{
  return _u_neighbor;
}

template <>
template <>
const VariableGradient &
MooseVariableFE<Real>::adGradSlnNeighbor<RESIDUAL>()
{
  return _grad_u_neighbor;
}

template <>
template <>
const VectorVariableGradient &
MooseVariableFE<RealVectorValue>::adGradSlnNeighbor<RESIDUAL>()
{
  return _grad_u_neighbor;
}

template <>
template <>
const VariableSecond &
MooseVariableFE<Real>::adSecondSlnNeighbor<RESIDUAL>()
{
  _need_second_neighbor = true;
  secondPhiFaceNeighbor();
  return _second_u_neighbor;
}

template <>
template <>
const VectorVariableSecond &
MooseVariableFE<RealVectorValue>::adSecondSlnNeighbor<RESIDUAL>()
{
  _need_second_neighbor = true;
  secondPhiFaceNeighbor();
  return _second_u_neighbor;
}

template <>
template <>
const VariableValue &
MooseVariableFE<Real>::adUDotNeighbor<RESIDUAL>()
{
  return _u_dot_neighbor;
}

template <>
template <>
const VectorVariableValue &
MooseVariableFE<RealVectorValue>::adUDotNeighbor<RESIDUAL>()
{
  return _u_dot_neighbor;
}

template <typename OutputType>
template <ComputeStage compute_stage>
const MooseArray<typename Moose::RealType<compute_stage>::type> &
MooseVariableFE<OutputType>::adDofValues()
{
  _need_ad = true;
  return _ad_dof_values;
}

template <>
template <>
const MooseArray<Real> &
MooseVariableFE<Real>::adDofValues<RESIDUAL>()
{
  return _dof_values;
}

template <>
template <>
const MooseArray<Real> &
MooseVariableFE<RealVectorValue>::adDofValues<RESIDUAL>()
{
  return _dof_values;
}

template <typename OutputType>
template <ComputeStage compute_stage>
const typename Moose::ValueType<OutputType, compute_stage>::type &
MooseVariableFE<OutputType>::adNodalValue()
{
  _need_ad = true;
  return _ad_nodal_value;
}

template <>
template <>
const Real &
MooseVariableFE<Real>::adNodalValue<RESIDUAL>()
{
  return _nodal_value;
}

template <>
template <>
const RealVectorValue &
MooseVariableFE<RealVectorValue>::adNodalValue<RESIDUAL>()
{
  return _nodal_value;
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiSecond &
MooseVariableFE<OutputType>::secondPhi()
{
  _second_phi = &_assembly.feSecondPhi<OutputType>(_fe_type);
  return *_second_phi;
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiCurl &
MooseVariableFE<OutputType>::curlPhi()
{
  _curl_phi = &_assembly.feCurlPhi<OutputType>(_fe_type);
  return *_curl_phi;
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiSecond &
MooseVariableFE<OutputType>::secondPhiFace()
{
  _second_phi_face = &_assembly.feSecondPhiFace<OutputType>(_fe_type);
  return *_second_phi_face;
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiCurl &
MooseVariableFE<OutputType>::curlPhiFace()
{
  _curl_phi_face = &_assembly.feCurlPhiFace<OutputType>(_fe_type);
  return *_curl_phi_face;
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiSecond &
MooseVariableFE<OutputType>::secondPhiNeighbor()
{
  _second_phi_neighbor = &_assembly.feSecondPhiNeighbor<OutputType>(_fe_type);
  return *_second_phi_neighbor;
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiCurl &
MooseVariableFE<OutputType>::curlPhiNeighbor()
{
  _curl_phi_neighbor = &_assembly.feCurlPhiNeighbor<OutputType>(_fe_type);
  return *_curl_phi_neighbor;
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiSecond &
MooseVariableFE<OutputType>::secondPhiFaceNeighbor()
{
  _second_phi_face_neighbor = &_assembly.feSecondPhiFaceNeighbor<OutputType>(_fe_type);
  return *_second_phi_face_neighbor;
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiCurl &
MooseVariableFE<OutputType>::curlPhiFaceNeighbor()
{
  _curl_phi_face_neighbor = &_assembly.feCurlPhiFaceNeighbor<OutputType>(_fe_type);
  return *_curl_phi_face_neighbor;
}

template class MooseVariableFE<Real>;
template class MooseVariableFE<RealVectorValue>;
template const MooseArray<DualReal> & MooseVariableFE<Real>::adDofValues<JACOBIAN>();
template const MooseArray<DualReal> & MooseVariableFE<RealVectorValue>::adDofValues<JACOBIAN>();
template const DualReal & MooseVariableFE<Real>::adNodalValue<JACOBIAN>();
template const libMesh::VectorValue<DualReal> &
MooseVariableFE<RealVectorValue>::adNodalValue<JACOBIAN>();
