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
    libmesh_assert(_dof_indices.size());
    _dof_values.resize(_dof_indices.size());
    _sys.currentSolution()->get(_dof_indices, &_dof_values[0]);

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

      libmesh_assert(_dof_indices_neighbor.size());
      _dof_values_neighbor.resize(_dof_indices_neighbor.size());
      _sys.currentSolution()->get(_dof_indices_neighbor, &_dof_values_neighbor[0]);

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
  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = qrule->n_points();
  auto safe_access_tagged_vectors = _sys.subproblem().safeAccessTaggedVectors();
  auto safe_access_tagged_matrices = _sys.subproblem().safeAccessTaggedMatrices();
  auto & active_coupleable_matrix_tags =
      _sys.subproblem().getActiveFEVariableCoupleableMatrixTags(_tid);
  auto & active_coupleable_vector_tags =
      _sys.subproblem().getActiveFEVariableCoupleableVectorTags(_tid);

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

  unsigned int num_dofs = _dof_indices.size();

  if (_need_dof_values)
    _dof_values.resize(num_dofs);

  if (_need_dof_values_previous_nl)
    _dof_values_previous_nl.resize(num_dofs);

  if (is_transient)
  {
    if (_need_dof_values_old)
      _dof_values_old.resize(num_dofs);
    if (_need_dof_values_older)
      _dof_values_older.resize(num_dofs);
    if (_need_dof_values_dot)
      _dof_values_dot.resize(num_dofs);
    if (_need_dof_values_dotdot)
      _dof_values_dotdot.resize(num_dofs);
    if (_need_dof_values_dot_old)
      _dof_values_dot_old.resize(num_dofs);
    if (_need_dof_values_dotdot_old)
      _dof_values_dotdot_old.resize(num_dofs);
  }

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old = _sys.solutionOld();
  const NumericVector<Real> & solution_older = _sys.solutionOlder();
  const NumericVector<Real> * solution_prev_nl = _sys.solutionPreviousNewton();
  const NumericVector<Real> * u_dot = _sys.solutionUDot();
  const NumericVector<Real> * u_dotdot = _sys.solutionUDotDot();
  const NumericVector<Real> * u_dot_old = _sys.solutionUDotOld();
  const NumericVector<Real> * u_dotdot_old = _sys.solutionUDotDotOld();
  const Real & du_dot_du = _sys.duDotDu();
  const Real & du_dotdot_du = _sys.duDotDotDu();

  dof_id_type idx = 0;
  Real soln_local = 0;
  Real tag_local_value = 0;
  Real soln_old_local = 0;
  Real soln_older_local = 0;
  Real soln_previous_nl_local = 0;
  Real u_dot_local = 0;
  Real u_dotdot_local = 0;
  Real u_dot_old_local = 0;
  Real u_dotdot_old_local = 0;

  const OutputType * phi_local = NULL;
  const typename OutputTools<OutputType>::OutputGradient * dphi_qp = NULL;
  const typename OutputTools<OutputType>::OutputSecond * d2phi_local = NULL;
  const OutputType * curl_phi_local = NULL;

  typename OutputTools<OutputType>::OutputGradient * grad_u_qp = NULL;
  typename OutputTools<OutputType>::OutputGradient * grad_u_old_qp = NULL;
  typename OutputTools<OutputType>::OutputGradient * grad_u_older_qp = NULL;
  typename OutputTools<OutputType>::OutputGradient * grad_u_previous_nl_qp = NULL;

  typename OutputTools<OutputType>::OutputSecond * second_u_qp = NULL;
  typename OutputTools<OutputType>::OutputSecond * second_u_old_qp = NULL;
  typename OutputTools<OutputType>::OutputSecond * second_u_older_qp = NULL;
  typename OutputTools<OutputType>::OutputSecond * second_u_previous_nl_qp = NULL;

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    idx = _dof_indices[i];
    soln_local = current_solution(idx);

    if (_need_dof_values)
      _dof_values[i] = soln_local;

    if (_need_u_previous_nl || _need_grad_previous_nl || _need_second_previous_nl ||
        _need_dof_values_previous_nl)
      soln_previous_nl_local = (*solution_prev_nl)(idx);

    if (_need_dof_values_previous_nl)
      _dof_values_previous_nl[i] = soln_previous_nl_local;

    if (is_transient)
    {
      if (_need_u_old || _need_grad_old || _need_second_old || _need_dof_values_old)
        soln_old_local = solution_old(idx);

      if (_need_u_older || _need_grad_older || _need_second_older || _need_dof_values_older)
        soln_older_local = solution_older(idx);

      if (_need_dof_values_old)
        _dof_values_old[i] = soln_old_local;
      if (_need_dof_values_older)
        _dof_values_older[i] = soln_older_local;

      if (u_dot)
      {
        u_dot_local = (*u_dot)(idx);
        if (_need_dof_values_dot)
          _dof_values_dot[i] = u_dot_local;
      }

      if (u_dotdot)
      {
        u_dotdot_local = (*u_dotdot)(idx);
        if (_need_dof_values_dotdot)
          _dof_values_dotdot[i] = u_dotdot_local;
      }

      if (u_dot_old)
      {
        u_dot_old_local = (*u_dot_old)(idx);
        if (_need_dof_values_dot_old)
          _dof_values_dot_old[i] = u_dot_old_local;
      }

      if (u_dotdot_old)
      {
        u_dotdot_old_local = (*u_dotdot_old)(idx);
        if (_need_dof_values_dotdot_old)
          _dof_values_dotdot_old[i] = u_dotdot_old_local;
      }
    }

    for (unsigned int qp = 0; qp < nqp; qp++)
    {
      phi_local = &phi[i][qp];
      dphi_qp = &grad_phi[i][qp];

      grad_u_qp = &_grad_u[qp];

      if (_need_grad_previous_nl)
        grad_u_previous_nl_qp = &_grad_u_previous_nl[qp];

      if (is_transient)
      {
        if (_need_grad_old)
          grad_u_old_qp = &_grad_u_old[qp];

        if (_need_grad_older)
          grad_u_older_qp = &_grad_u_older[qp];
      }

      if (_need_second || _need_second_old || _need_second_older || _need_second_previous_nl)
      {
        d2phi_local = &(*second_phi)[i][qp];

        if (_need_second)
        {
          second_u_qp = &_second_u[qp];
          second_u_qp->add_scaled(*d2phi_local, soln_local);
        }

        if (_need_second_previous_nl)
        {
          second_u_previous_nl_qp = &_second_u_previous_nl[qp];
          second_u_previous_nl_qp->add_scaled(*d2phi_local, soln_previous_nl_local);
        }

        if (is_transient)
        {
          if (_need_second_old)
            second_u_old_qp = &_second_u_old[qp];

          if (_need_second_older)
            second_u_older_qp = &_second_u_older[qp];
        }
      }

      if (_need_curl || _need_curl_old)
      {
        curl_phi_local = &(*curl_phi)[i][qp];

        if (_need_curl)
          _curl_u[qp] += *curl_phi_local * soln_local;

        if (is_transient && _need_curl_old)
          _curl_u_old[qp] += *curl_phi_local * soln_old_local;
      }

      _u[qp] += *phi_local * soln_local;

      if (safe_access_tagged_vectors)
      {
        for (auto tag : active_coupleable_vector_tags)
          if (_need_vector_tag_u[tag] && _sys.hasVector(tag) && _sys.getVector(tag).closed())
          {
            tag_local_value = _sys.getVector(tag)(idx);
            _vector_tag_u[tag][qp] += *phi_local * tag_local_value;
          }
      }

      if (safe_access_tagged_matrices)
      {
        for (auto tag : active_coupleable_matrix_tags)
          if (_need_matrix_tag_u[tag] && _sys.hasMatrix(tag) && _sys.getMatrix(tag).closed())
          {
            Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
            tag_local_value = _sys.getMatrix(tag)(idx, idx);
            _matrix_tag_u[tag][qp] += *phi_local * tag_local_value;
          }
      }

      grad_u_qp->add_scaled(*dphi_qp, soln_local);

      if (_need_u_previous_nl)
        _u_previous_nl[qp] += *phi_local * soln_previous_nl_local;
      if (_need_grad_previous_nl)
        grad_u_previous_nl_qp->add_scaled(*dphi_qp, soln_previous_nl_local);

      if (is_transient)
      {
        if (_need_u_dot)
          _u_dot[qp] += *phi_local * u_dot_local;

        if (_need_u_dotdot)
          _u_dotdot[qp] += *phi_local * u_dotdot_local;

        if (_need_u_dot_old)
          _u_dot_old[qp] += *phi_local * u_dot_old_local;

        if (_need_u_dotdot_old)
          _u_dotdot_old[qp] += *phi_local * u_dotdot_old_local;

        if (_need_du_dot_du)
          _du_dot_du[qp] = du_dot_du;

        if (_need_du_dotdot_du)
          _du_dotdot_du[qp] = du_dotdot_du;

        if (_need_grad_dot)
          _grad_u_dot[qp].add_scaled(*dphi_qp, u_dot_local);

        if (_need_grad_dotdot)
          _grad_u_dotdot[qp].add_scaled(*dphi_qp, u_dotdot_local);

        if (_need_u_old)
          _u_old[qp] += *phi_local * soln_old_local;

        if (_need_u_older)
          _u_older[qp] += *phi_local * soln_older_local;

        if (_need_grad_old)
          grad_u_old_qp->add_scaled(*dphi_qp, soln_old_local);

        if (_need_grad_older)
          grad_u_older_qp->add_scaled(*dphi_qp, soln_older_local);

        if (_need_second_old)
          second_u_old_qp->add_scaled(*d2phi_local, soln_old_local);

        if (_need_second_older)
          second_u_older_qp->add_scaled(*d2phi_local, soln_older_local);
      }
    }
  }

  // Automatic differentiation
  if (_need_ad && _subproblem.currentlyComputingJacobian())
    computeAD(num_dofs, nqp, is_transient, phi, grad_phi, second_phi, ad_grad_phi);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeAD(
    const unsigned int & num_dofs,
    const unsigned int & nqp,
    const bool & is_transient,
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

  if (is_transient)
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

    if (is_transient)
      _ad_u_dot[qp] = _ad_zero;
  }

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    _ad_dof_values[i] = (*_sys.currentSolution())(_dof_indices[i]);

    // NOTE!  You have to do this AFTER setting the value!
    if (_var_kind == Moose::VAR_NONLINEAR)
      _ad_dof_values[i].derivatives()[ad_offset + i] = 1.0;

    if (is_transient && _time_integrator)
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

      if (is_transient)
        _ad_u_dot[qp] += phi[i][qp] * _ad_dofs_dot[i];
    }
  }
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeADNeighbor(const unsigned int & num_dofs,
                                               const unsigned int & nqp,
                                               const bool & is_transient,
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

    if (is_transient)
    {
      _neighbor_ad_dofs_dot[qp] = _ad_zero;
      _neighbor_ad_u_dot[qp] = _ad_zero;
    }
  }

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    _neighbor_ad_dof_values[i] = (*_sys.currentSolution())(_dof_indices_neighbor[i]);

    // NOTE!  You have to do this AFTER setting the value!
    if (_var_kind == Moose::VAR_NONLINEAR)
      _neighbor_ad_dof_values[i].derivatives()[ad_offset + i] = 1.0;

    if (is_transient && _time_integrator)
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

      if (is_transient)
        _neighbor_ad_u_dot[qp] += phi[i][qp] * _neighbor_ad_dofs_dot[i];
    }
  }
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeNeighborValuesHelper(QBase *& qrule,
                                                         const FieldVariablePhiValue & phi,
                                                         const FieldVariablePhiGradient & grad_phi,
                                                         const FieldVariablePhiSecond *& second_phi)
{
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

  unsigned int num_dofs = _dof_indices_neighbor.size();

  if (_need_dof_values_neighbor)
    _dof_values_neighbor.resize(num_dofs);
  if (is_transient)
  {
    if (_need_dof_values_old_neighbor)
      _dof_values_old_neighbor.resize(num_dofs);
    if (_need_dof_values_older_neighbor)
      _dof_values_older_neighbor.resize(num_dofs);
    if (_need_dof_values_dot_neighbor)
      _dof_values_dot_neighbor.resize(num_dofs);
    if (_need_dof_values_dotdot_neighbor)
      _dof_values_dotdot_neighbor.resize(num_dofs);
    if (_need_dof_values_dot_old_neighbor)
      _dof_values_dot_old_neighbor.resize(num_dofs);
    if (_need_dof_values_dotdot_old_neighbor)
      _dof_values_dotdot_old_neighbor.resize(num_dofs);
  }

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old = _sys.solutionOld();
  const NumericVector<Real> & solution_older = _sys.solutionOlder();
  const NumericVector<Real> * u_dot = _sys.solutionUDot();
  const NumericVector<Real> * u_dotdot = _sys.solutionUDotDot();
  const NumericVector<Real> * u_dot_old = _sys.solutionUDotOld();
  const NumericVector<Real> * u_dotdot_old = _sys.solutionUDotDotOld();
  const Real & du_dot_du = _sys.duDotDu();
  const Real & du_dotdot_du = _sys.duDotDotDu();

  dof_id_type idx;
  Real soln_local;
  Real soln_old_local = 0;
  Real soln_older_local = 0;
  Real u_dot_local = 0;
  Real u_dotdot_local = 0;
  Real u_dot_old_local = 0;
  Real u_dotdot_old_local = 0;

  OutputType phi_local;
  typename OutputTools<OutputType>::OutputGradient dphi_local;
  typename OutputTools<OutputType>::OutputSecond d2phi_local;

  for (unsigned int i = 0; i < num_dofs; ++i)
  {
    idx = _dof_indices_neighbor[i];
    soln_local = current_solution(idx);

    if (_need_dof_values_neighbor)
      _dof_values_neighbor[i] = soln_local;

    if (is_transient)
    {
      if (_need_u_old_neighbor || _need_dof_values_old_neighbor)
        soln_old_local = solution_old(idx);

      if (_need_u_older_neighbor || _need_dof_values_older_neighbor)
        soln_older_local = solution_older(idx);

      if (_need_dof_values_old_neighbor)
        _dof_values_old_neighbor[i] = soln_old_local;
      if (_need_dof_values_older_neighbor)
        _dof_values_older_neighbor[i] = soln_older_local;

      if (u_dot)
      {
        u_dot_local = (*u_dot)(idx);
        if (_need_dof_values_dot_neighbor)
          _dof_values_dot_neighbor[i] = u_dot_local;
      }

      if (u_dotdot)
      {
        u_dotdot_local = (*u_dotdot)(idx);
        if (_need_dof_values_dotdot_neighbor)
          _dof_values_dotdot_neighbor[i] = u_dotdot_local;
      }

      if (u_dot_old)
      {
        u_dot_old_local = (*u_dot_old)(idx);
        if (_need_dof_values_dot_old_neighbor)
          _dof_values_dot_old_neighbor[i] = u_dot_old_local;
      }

      if (u_dotdot_old)
      {
        u_dotdot_old_local = (*u_dotdot_old)(idx);
        if (_need_dof_values_dotdot_old_neighbor)
          _dof_values_dotdot_old_neighbor[i] = u_dotdot_old_local;
      }
    }

    for (unsigned int qp = 0; qp < nqp; ++qp)
    {
      phi_local = phi[i][qp];
      dphi_local = grad_phi[i][qp];

      if (_need_second_neighbor || _need_second_old_neighbor || _need_second_older_neighbor)
        d2phi_local = (*second_phi)[i][qp];

      _u_neighbor[qp] += phi_local * soln_local;
      _grad_u_neighbor[qp] += dphi_local * soln_local;

      if (_need_second_neighbor)
        _second_u_neighbor[qp] += d2phi_local * soln_local;

      if (is_transient)
      {
        if (_need_u_dot_neighbor)
          _u_dot_neighbor[qp] += phi_local * u_dot_local;

        if (_need_u_dotdot_neighbor)
          _u_dotdot_neighbor[qp] += phi_local * u_dotdot_local;

        if (_need_u_dot_old_neighbor)
          _u_dot_old_neighbor[qp] += phi_local * u_dot_old_local;

        if (_need_u_dotdot_old_neighbor)
          _u_dotdot_old_neighbor[qp] += phi_local * u_dotdot_old_local;

        if (_need_du_dot_du_neighbor)
          _du_dot_du_neighbor[qp] = du_dot_du;

        if (_need_du_dotdot_du_neighbor)
          _du_dotdot_du_neighbor[qp] = du_dotdot_du;

        if (_need_grad_neighbor_dot)
          _grad_u_neighbor_dot[qp].add_scaled(dphi_local, u_dot_local);

        if (_need_grad_neighbor_dotdot)
          _grad_u_neighbor_dotdot[qp].add_scaled(dphi_local, u_dotdot_local);

        if (_need_u_old_neighbor)
          _u_old_neighbor[qp] += phi_local * soln_old_local;

        if (_need_u_older_neighbor)
          _u_older_neighbor[qp] += phi_local * soln_older_local;

        if (_need_grad_old_neighbor)
          _grad_u_old_neighbor[qp] += dphi_local * soln_old_local;

        if (_need_grad_older_neighbor)
          _grad_u_older_neighbor[qp] += dphi_local * soln_older_local;

        if (_need_second_old_neighbor)
          _second_u_old_neighbor[qp] += d2phi_local * soln_old_local;

        if (_need_second_older_neighbor)
          _second_u_older_neighbor[qp] += d2phi_local * soln_older_local;
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
  auto safe_access_tagged_vectors = _sys.subproblem().safeAccessTaggedVectors();
  auto safe_access_tagged_matrices = _sys.subproblem().safeAccessTaggedMatrices();
  auto & active_coupleable_matrix_tags =
      _sys.subproblem().getActiveFEVariableCoupleableMatrixTags(_tid);
  auto & active_coupleable_vector_tags =
      _sys.subproblem().getActiveFEVariableCoupleableVectorTags(_tid);

  if (_has_dof_indices)
  {
    auto n = _dof_indices.size();
    mooseAssert(n, "There must be a non-zero number of degrees of freedom");
    _dof_values.resize(n);

    _sys.currentSolution()->get(_dof_indices, &_dof_values[0]);
    for (decltype(n) i = 0; i < n; ++i)
      assignNodalValue(_dof_values[i], i);

    for (auto tag : active_coupleable_vector_tags)
      _vector_tags_dof_u[tag].resize(n);

    for (auto tag : active_coupleable_matrix_tags)
      _matrix_tags_dof_u[tag].resize(n);

    if (safe_access_tagged_vectors)
    {
      for (auto tag : active_coupleable_vector_tags)
        if (_need_vector_tag_dof_u[tag] && _sys.hasVector(tag) && _sys.getVector(tag).closed())
        {
          auto & vec = _sys.getVector(tag);
          vec.get(_dof_indices, &_vector_tags_dof_u[tag][0]);
        }
    }

    if (safe_access_tagged_matrices)
    {
      for (auto tag : active_coupleable_matrix_tags)
        if (_need_matrix_tag_dof_u[tag] && _sys.hasMatrix(tag) && _sys.matrixTagActive(tag) &&
            _sys.getMatrix(tag).closed())
        {
          auto & mat = _sys.getMatrix(tag);
          for (unsigned i = 0; i < _dof_indices.size(); i++)
          {
            Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
            _matrix_tags_dof_u[tag][i] = mat(_dof_indices[i], _dof_indices[i]);
          }
        }
    }

    if (_need_dof_values_previous_nl)
    {
      _dof_values_previous_nl.resize(n);
      _sys.solutionPreviousNewton()->get(_dof_indices, &_dof_values_previous_nl[0]);
      for (decltype(n) i = 0; i < n; ++i)
        assignNodalValuePreviousNL(_dof_values_previous_nl[i], i);
    }
    if (_subproblem.isTransient())
    {
      _dof_values_old.resize(n);
      _dof_values_older.resize(n);
      _sys.solutionOld().get(_dof_indices, &_dof_values_old[0]);
      _sys.solutionOlder().get(_dof_indices, &_dof_values_older[0]);
      for (decltype(n) i = 0; i < n; ++i)
      {
        assignNodalValueOld(_dof_values_old[i], i);
        assignNodalValueOlder(_dof_values_older[i], i);
      }

      _dof_values_dot.resize(n);
      _dof_values_dotdot.resize(n);
      _dof_values_dot_old.resize(n);
      _dof_values_dotdot_old.resize(n);
      _dof_du_dot_du.resize(n);
      _dof_du_dotdot_du.resize(n);

      if (_sys.solutionUDot())
        _dof_values_dot[0] = (*_sys.solutionUDot())(_dof_indices[0]);
      if (_sys.solutionUDotDot())
        _dof_values_dotdot[0] = (*_sys.solutionUDotDot())(_dof_indices[0]);
      if (_sys.solutionUDotOld())
        _dof_values_dot_old[0] = (*_sys.solutionUDotOld())(_dof_indices[0]);
      if (_sys.solutionUDotDotOld())
        _dof_values_dotdot_old[0] = (*_sys.solutionUDotDotOld())(_dof_indices[0]);
      _dof_du_dot_du[0] = _sys.duDotDu();
      _dof_du_dotdot_du[0] = _sys.duDotDotDu();

      for (decltype(n) i = 0; i < n; ++i)
      {
        assignNodalValueDot(_dof_values_dot[i], i);
        assignNodalValueDotDot(_dof_values_dotdot[i], i);
        assignNodalValueDotOld(_dof_values_dot_old[i], i);
        assignNodalValueDotDotOld(_dof_values_dotdot_old[i], i);
      }
    }

    if (_need_ad && _subproblem.currentlyComputingJacobian())
    {
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
  }
  else
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
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeNodalNeighborValues()
{
  if (_neighbor_has_dof_indices)
  {
    const unsigned int n = _dof_indices_neighbor.size();
    mooseAssert(n, "There must be a non-zero number of degrees of freedom");
    _dof_values_neighbor.resize(n);
    _sys.currentSolution()->get(_dof_indices_neighbor, &_dof_values_neighbor[0]);
    for (unsigned int i = 0; i < n; ++i)
      assignNeighborNodalValue(_dof_values_neighbor[i], i);

    if (_need_dof_values_previous_nl_neighbor)
    {
      _dof_values_previous_nl_neighbor.resize(n);
      _sys.solutionPreviousNewton()->get(_dof_indices_neighbor,
                                         &_dof_values_previous_nl_neighbor[0]);
      for (unsigned int i = 0; i < n; ++i)
        assignNeighborNodalValuePreviousNL(_dof_values_previous_nl_neighbor[i], i);
    }
    if (_subproblem.isTransient())
    {
      _dof_values_old_neighbor.resize(n);
      _dof_values_older_neighbor.resize(n);
      _sys.solutionOld().get(_dof_indices_neighbor, &_dof_values_old_neighbor[0]);
      _sys.solutionOlder().get(_dof_indices_neighbor, &_dof_values_older_neighbor[0]);
      for (unsigned int i = 0; i < n; ++i)
      {
        assignNeighborNodalValueOld(_dof_values_old_neighbor[i], i);
        assignNeighborNodalValueOlder(_dof_values_older_neighbor[i], i);
      }

      _dof_values_dot_neighbor.resize(n);
      _dof_values_dotdot_neighbor.resize(n);
      _dof_values_dot_old_neighbor.resize(n);
      _dof_values_dotdot_old_neighbor.resize(n);
      _dof_du_dot_du_neighbor.resize(n);
      _dof_du_dotdot_du_neighbor.resize(n);
      for (unsigned int i = 0; i < n; i++)
      {
        if (_sys.solutionUDot())
          _dof_values_dot_neighbor[i] = (*_sys.solutionUDot())(_dof_indices_neighbor[i]);
        if (_sys.solutionUDotDot())
          _dof_values_dotdot_neighbor[i] = (*_sys.solutionUDotDot())(_dof_indices_neighbor[i]);
        if (_sys.solutionUDotOld())
          _dof_values_dot_old_neighbor[i] = (*_sys.solutionUDotOld())(_dof_indices_neighbor[i]);
        if (_sys.solutionUDotDotOld())
          _dof_values_dotdot_old_neighbor[i] =
              (*_sys.solutionUDotDotOld())(_dof_indices_neighbor[i]);
        _dof_du_dot_du_neighbor[i] = _sys.duDotDu();
        _dof_du_dotdot_du_neighbor[i] = _sys.duDotDotDu();
      }
    }
  }
  else
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
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::assignNodalValue(const Real & value, const unsigned int &)
{
  _nodal_value = value;
}

template <>
void
MooseVariableFE<RealVectorValue>::assignNodalValue(const Real & value,
                                                   const unsigned int & component)
{
  _nodal_value(component) = value;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::assignADNodalValue(const DualReal & value, const unsigned int &)
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
MooseVariableFE<OutputType>::assignNodalValueOld(const Real & value, const unsigned int &)
{
  _nodal_value_old = value;
}

template <>
void
MooseVariableFE<RealVectorValue>::assignNodalValueOld(const Real & value,
                                                      const unsigned int & component)
{
  _nodal_value_old(component) = value;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::assignNodalValueOlder(const Real & value, const unsigned int &)
{
  _nodal_value_older = value;
}

template <>
void
MooseVariableFE<RealVectorValue>::assignNodalValueOlder(const Real & value,
                                                        const unsigned int & component)
{
  _nodal_value_older(component) = value;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::assignNodalValuePreviousNL(const Real & value, const unsigned int &)
{
  _nodal_value_previous_nl = value;
}

template <>
void
MooseVariableFE<RealVectorValue>::assignNodalValuePreviousNL(const Real & value,
                                                             const unsigned int & component)
{
  _nodal_value_previous_nl(component) = value;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::assignNodalValueDot(const Real & value, const unsigned int &)
{
  _nodal_value_dot = value;
}

template <>
void
MooseVariableFE<RealVectorValue>::assignNodalValueDot(const Real & value,
                                                      const unsigned int & component)
{
  _nodal_value_dot(component) = value;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::assignNodalValueDotOld(const Real & value, const unsigned int &)
{
  _nodal_value_dot_old = value;
}

template <>
void
MooseVariableFE<RealVectorValue>::assignNodalValueDotOld(const Real & value,
                                                         const unsigned int & component)
{
  _nodal_value_dot_old(component) = value;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::assignNodalValueDotDot(const Real & value, const unsigned int &)
{
  _nodal_value_dotdot = value;
}

template <>
void
MooseVariableFE<RealVectorValue>::assignNodalValueDotDot(const Real & value,
                                                         const unsigned int & component)
{
  _nodal_value_dotdot(component) = value;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::assignNodalValueDotDotOld(const Real & value, const unsigned int &)
{
  _nodal_value_dotdot_old = value;
}

template <>
void
MooseVariableFE<RealVectorValue>::assignNodalValueDotDotOld(const Real & value,
                                                            const unsigned int & component)
{
  _nodal_value_dotdot_old(component) = value;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::assignNeighborNodalValue(const Real & value, const unsigned int &)
{
  _neighbor_nodal_value = value;
}

template <>
void
MooseVariableFE<RealVectorValue>::assignNeighborNodalValue(const Real & value,
                                                           const unsigned int & component)
{
  _neighbor_nodal_value(component) = value;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::assignNeighborNodalValueOld(const Real & value, const unsigned int &)
{
  _neighbor_nodal_value_old = value;
}

template <>
void
MooseVariableFE<RealVectorValue>::assignNeighborNodalValueOld(const Real & value,
                                                              const unsigned int & component)
{
  _neighbor_nodal_value_old(component) = value;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::assignNeighborNodalValueOlder(const Real & value, const unsigned int &)
{
  _neighbor_nodal_value_older = value;
}

template <>
void
MooseVariableFE<RealVectorValue>::assignNeighborNodalValueOlder(const Real & value,
                                                                const unsigned int & component)
{
  _neighbor_nodal_value_older(component) = value;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::assignNeighborNodalValuePreviousNL(const Real & value,
                                                                const unsigned int &)
{
  _neighbor_nodal_value_previous_nl = value;
}

template <>
void
MooseVariableFE<RealVectorValue>::assignNeighborNodalValuePreviousNL(const Real & value,
                                                                     const unsigned int & component)
{
  _neighbor_nodal_value_previous_nl(component) = value;
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
  assignNodalValue(value, 0);

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
    assignNodalValue(value(i), i);
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
