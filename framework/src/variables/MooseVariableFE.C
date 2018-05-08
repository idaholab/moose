//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableFE.h"

template <typename OutputType>
MooseVariableFE<OutputType>::MooseVariableFE(unsigned int var_num,
                                             const FEType & fe_type,
                                             SystemBase & sys,
                                             Assembly & assembly,
                                             Moose::VarKindType var_kind)
  : MooseVariableFEBase(var_num, fe_type, sys, var_kind),
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
    _need_grad_old(false),
    _need_grad_older(false),
    _need_grad_previous_nl(false),
    _need_grad_dot(false),
    _need_second(false),
    _need_second_old(false),
    _need_second_older(false),
    _need_second_previous_nl(false),
    _need_curl(false),
    _need_curl_old(false),
    _need_curl_older(false),
    _need_u_old_neighbor(false),
    _need_u_older_neighbor(false),
    _need_u_previous_nl_neighbor(false),
    _need_grad_old_neighbor(false),
    _need_grad_older_neighbor(false),
    _need_grad_previous_nl_neighbor(false),
    _need_grad_neighbor_dot(false),
    _need_second_neighbor(false),
    _need_second_old_neighbor(false),
    _need_second_older_neighbor(false),
    _need_second_previous_nl_neighbor(false),
    _need_curl_neighbor(false),
    _need_curl_old_neighbor(false),
    _need_curl_older_neighbor(false),
    _need_solution_dofs(false),
    _need_solution_dofs_old(false),
    _need_solution_dofs_older(false),
    _need_solution_dofs_neighbor(false),
    _need_solution_dofs_old_neighbor(false),
    _need_solution_dofs_older_neighbor(false),
    _need_dof_values(false),
    _need_dof_values_old(false),
    _need_dof_values_older(false),
    _need_dof_values_previous_nl(false),
    _need_dof_values_dot(false),
    _need_dof_du_dot_du(false),
    _need_dof_values_neighbor(false),
    _need_dof_values_old_neighbor(false),
    _need_dof_values_older_neighbor(false),
    _need_dof_values_previous_nl_neighbor(false),
    _need_dof_values_dot_neighbor(false),
    _need_dof_du_dot_du_neighbor(false),
    _need_vector_tag_dof_u(false),
    _need_matrix_tag_dof_u(false),
    _normals(_assembly.normals()),
    _is_nodal(true),
    _has_dofs(false),
    _neighbor_has_dofs(false),
    _has_nodal_value(false),
    _has_nodal_value_neighbor(false),
    _node(_assembly.node()),
    _node_neighbor(_assembly.nodeNeighbor()),
    _phi(_assembly.fePhi<OutputType>(_fe_type)),
    _grad_phi(_assembly.feGradPhi<OutputType>(_fe_type)),
    _phi_face(_assembly.fePhiFace<OutputType>(_fe_type)),
    _grad_phi_face(_assembly.feGradPhiFace<OutputType>(_fe_type)),
    _phi_neighbor(_assembly.fePhiNeighbor<OutputType>(_fe_type)),
    _grad_phi_neighbor(_assembly.feGradPhiNeighbor<OutputType>(_fe_type)),
    _phi_face_neighbor(_assembly.fePhiFaceNeighbor<OutputType>(_fe_type)),
    _grad_phi_face_neighbor(_assembly.feGradPhiFaceNeighbor<OutputType>(_fe_type))
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

  auto num_matrix_tags = _sys.subproblem().numMatrixTags();

  _matrix_tags_dof_u.resize(num_matrix_tags);
}

template <typename OutputType>
MooseVariableFE<OutputType>::~MooseVariableFE()
{
  _dof_values.release();
  _dof_values_old.release();
  _dof_values_older.release();
  _dof_values_previous_nl.release();
  _dof_values_dot.release();
  _dof_du_dot_du.release();

  _dof_values_neighbor.release();
  _dof_values_old_neighbor.release();
  _dof_values_older_neighbor.release();
  _dof_values_previous_nl_neighbor.release();
  _dof_values_dot_neighbor.release();
  _dof_du_dot_du_neighbor.release();

  for (auto & dof_u : _vector_tags_dof_u)
    dof_u.release();

  _vector_tags_dof_u.clear();

  for (auto & dof_u : _matrix_tags_dof_u)
    dof_u.release();

  _matrix_tags_dof_u.clear();

  _u.release();
  _u_bak.release();
  _u_old.release();
  _u_old_bak.release();
  _u_older.release();
  _u_older_bak.release();
  _u_previous_nl.release();

  _grad_u.release();
  _grad_u_bak.release();
  _grad_u_old.release();
  _grad_u_old_bak.release();
  _grad_u_older.release();
  _grad_u_older.release();
  _grad_u_previous_nl.release();
  _grad_u_dot.release();

  _second_u.release();
  _second_u_bak.release();
  _second_u_old.release();
  _second_u_old_bak.release();
  _second_u_older.release();
  _second_u_older_bak.release();
  _second_u_previous_nl.release();

  _curl_u.release();
  _curl_u_bak.release();
  _curl_u_old.release();
  _curl_u_old_bak.release();
  _curl_u_older.release();

  _u_dot.release();
  _u_dot_bak.release();
  _u_dot_neighbor.release();
  _u_dot_bak_neighbor.release();

  _du_dot_du.release();
  _du_dot_du_bak.release();
  _du_dot_du_neighbor.release();
  _du_dot_du_bak_neighbor.release();

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

  _second_u_neighbor.release();
  _second_u_old_neighbor.release();
  _second_u_older_neighbor.release();
  _second_u_previous_nl_neighbor.release();

  _curl_u_neighbor.release();
  _curl_u_old_neighbor.release();
  _curl_u_older_neighbor.release();
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
  _has_nodal_value = false;
  _has_nodal_value_neighbor = false;

  // FIXME: remove this when the Richard's module is migrated to use the new NodalCoupleable
  // interface.
  if (_dof_indices.size() > 0)
    _has_dofs = true;
  else
    _has_dofs = false;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::prepareNeighbor()
{
  _dof_map.dof_indices(_neighbor, _dof_indices_neighbor, _var_num);
  _has_nodal_value = false;
  _has_nodal_value_neighbor = false;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::prepareAux()
{
  _has_nodal_value = false;
  _has_nodal_value_neighbor = false;
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
    _has_dofs = true;
  }
  else
  {
    _dof_indices.clear(); // Clear these so Assembly doesn't think there's dofs here
    _has_dofs = false;
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

    _has_dofs = true;
  }
  else
    _has_dofs = false;
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

      _neighbor_has_dofs = true;
    }
    else
      _neighbor_has_dofs = false;
  }
  else
    _neighbor_has_dofs = false;
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
    _has_dofs = true;
  else
    _has_dofs = false;
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
    _neighbor_has_dofs = true;
  else
    _neighbor_has_dofs = false;
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
  if (_has_nodal_value)
    residual.insert(&_dof_values[0], _dof_indices);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::add(NumericVector<Number> & residual)
{
  if (_has_nodal_value)
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
  _need_dof_values_dot = true;
  return _dof_values_dot;
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotNeighbor()
{
  _need_dof_values_dot_neighbor = true;
  return _dof_values_dot_neighbor;
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
MooseVariableFE<OutputType>::dofValuesDuDotDuNeighbor()
{
  _need_dof_du_dot_du_neighbor = true;
  return _dof_du_dot_du_neighbor;
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
  computeValuesHelper(_qrule, _phi, _grad_phi, _second_phi, _curl_phi);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeElemValuesFace()
{
  computeValuesHelper(_qrule_face, _phi_face, _grad_phi_face, _second_phi_face, _curl_phi_face);
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
MooseVariableFE<OutputType>::setNodalValue(const DenseVector<Number> & values)
{
  for (unsigned int i = 0; i < values.size(); i++)
    _dof_values[i] = values(i);

  _has_nodal_value = true;

  for (unsigned int qp = 0; qp < _u.size(); qp++)
  {
    _u[qp] = 0;
    for (unsigned int i = 0; i < _dof_values.size(); i++)
      _u[qp] += _phi[i][qp] * _dof_values[i];
  }
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::setNodalValue(Number value, unsigned int idx /* = 0*/)
{
  _dof_values[idx] = value; // update variable nodal value
  _has_nodal_value = true;

  // Update the qp values as well
  for (unsigned int qp = 0; qp < _u.size(); qp++)
    _u[qp] = value;
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
MooseVariableFE<OutputType>::computeValuesHelper(QBase *& qrule,
                                                 const FieldVariablePhiValue & phi,
                                                 const FieldVariablePhiGradient & grad_phi,
                                                 const FieldVariablePhiSecond *& second_phi,
                                                 const FieldVariablePhiValue *& curl_phi)

{
  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = qrule->n_points();

  _u.resize(nqp);
  _grad_u.resize(nqp);

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
    _u_dot.resize(nqp);
    _du_dot_du.resize(nqp);

    if (_need_grad_dot)
      _grad_u_dot.resize(nqp);

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
      _u_dot[i] = 0;
      _du_dot_du[i] = 0;

      if (_need_grad_dot)
        _grad_u_dot[i] = 0;

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
  }

  if (_need_solution_dofs)
    _solution_dofs.resize(num_dofs);

  if (_need_solution_dofs_old)
    _solution_dofs_old.resize(num_dofs);

  if (_need_solution_dofs_older)
    _solution_dofs_older.resize(num_dofs);

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old = _sys.solutionOld();
  const NumericVector<Real> & solution_older = _sys.solutionOlder();
  const NumericVector<Real> * solution_prev_nl = _sys.solutionPreviousNewton();
  const NumericVector<Real> & u_dot = _sys.solutionUDot();
  const Real & du_dot_du = _sys.duDotDu();

  dof_id_type idx = 0;
  Real soln_local = 0;
  Real soln_old_local = 0;
  Real soln_older_local = 0;
  Real soln_previous_nl_local = 0;
  Real u_dot_local = 0;

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

    if (_need_solution_dofs)
      _solution_dofs(i) = soln_local;

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

      u_dot_local = u_dot(idx);
      if (_need_dof_values_dot)
        _dof_values_dot[i] = u_dot_local;

      if (_need_solution_dofs_old)
        _solution_dofs_old(i) = solution_old(idx);

      if (_need_solution_dofs_older)
        _solution_dofs_older(i) = solution_older(idx);
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

      grad_u_qp->add_scaled(*dphi_qp, soln_local);

      if (_need_u_previous_nl)
        _u_previous_nl[qp] += *phi_local * soln_previous_nl_local;
      if (_need_grad_previous_nl)
        grad_u_previous_nl_qp->add_scaled(*dphi_qp, soln_previous_nl_local);

      if (is_transient)
      {
        _u_dot[qp] += *phi_local * u_dot_local;
        _du_dot_du[qp] = du_dot_du;

        if (_need_grad_dot)
          _grad_u_dot[qp].add_scaled(*dphi_qp, u_dot_local);

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
    _u_dot_neighbor.resize(nqp);
    _du_dot_du_neighbor.resize(nqp);
    if (_need_grad_neighbor_dot)
      _grad_u_neighbor_dot.resize(nqp);

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

    if (is_transient)
    {
      _u_dot_neighbor[i] = 0;
      _du_dot_du_neighbor[i] = 0;

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
  }

  if (_need_solution_dofs_neighbor)
    _solution_dofs_neighbor.resize(num_dofs);

  if (_need_solution_dofs_old_neighbor)
    _solution_dofs_old_neighbor.resize(num_dofs);

  if (_need_solution_dofs_older_neighbor)
    _solution_dofs_older_neighbor.resize(num_dofs);

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old = _sys.solutionOld();
  const NumericVector<Real> & solution_older = _sys.solutionOlder();
  const NumericVector<Real> & u_dot = _sys.solutionUDot();
  const Real & du_dot_du = _sys.duDotDu();

  dof_id_type idx;
  Real soln_local;
  Real soln_old_local = 0;
  Real soln_older_local = 0;
  Real u_dot_local = 0;

  OutputType phi_local;
  typename OutputTools<OutputType>::OutputGradient dphi_local;
  typename OutputTools<OutputType>::OutputSecond d2phi_local;

  for (unsigned int i = 0; i < num_dofs; ++i)
  {
    idx = _dof_indices_neighbor[i];
    soln_local = current_solution(idx);

    if (_need_dof_values_neighbor)
      _dof_values_neighbor[i] = soln_local;

    if (_need_solution_dofs_neighbor)
      _solution_dofs_neighbor(i) = soln_local;

    if (is_transient)
    {
      if (_need_u_old_neighbor)
        soln_old_local = solution_old(idx);

      if (_need_u_older_neighbor)
        soln_older_local = solution_older(idx);

      if (_need_dof_values_old_neighbor)
        _dof_values_old_neighbor[i] = soln_old_local;
      if (_need_dof_values_older_neighbor)
        _dof_values_older_neighbor[i] = soln_older_local;

      u_dot_local = u_dot(idx);
      if (_need_dof_values_dot_neighbor)
        _dof_values_dot_neighbor[i] = u_dot_local;

      if (_need_solution_dofs_old_neighbor)
        _solution_dofs_old_neighbor(i) = solution_old(idx);

      if (_need_solution_dofs_older_neighbor)
        _solution_dofs_older_neighbor(i) = solution_older(idx);
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
        _u_dot_neighbor[qp] += phi_local * u_dot_local;
        _du_dot_du_neighbor[qp] = du_dot_du;

        if (_need_grad_neighbor_dot)
          _grad_u_neighbor_dot[qp].add_scaled(dphi_local, u_dot_local);

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
const MooseArray<Real> &
MooseVariableFE<OutputType>::nodalVectorTagValue(TagID tag)
{
  if (isNodal())
  {
    _need_vector_tag_dof_u = true;

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
    _need_matrix_tag_dof_u = true;

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
MooseVariableFE<OutputType>::nodalValueDot()
{
  if (isNodal())
  {
    _need_dof_values_dot = true;
    return _nodal_value_dot;
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
  if (_has_dofs)
  {
    const size_t n = _dof_indices.size();
    mooseAssert(n, "There must be a non-zero number of degrees of freedom");
    _dof_values.resize(n);
    _sys.currentSolution()->get(_dof_indices, &_dof_values[0]);
    _nodal_value = _dof_values[0];

    if (_need_vector_tag_dof_u)
    {
      TagID tag = 0;
      for (auto & dof_u : _vector_tags_dof_u)
      {
        if (_sys.hasVector(tag))
        {
          dof_u.resize(n);
          auto & vec = _sys.getVector(tag);
          vec.get(_dof_indices, &dof_u[0]);
        }
        tag++;
      }
    }

    if (_need_matrix_tag_dof_u)
    {
      TagID tag = 0;
      for (auto & dof_u : _matrix_tags_dof_u)
      {
        if (_sys.hasMatrix(tag) && _sys.matrixTagActive(tag))
        {
          dof_u.resize(n);
          auto & mat = _sys.getMatrix(tag);
          for (unsigned i = 0; i < _dof_indices.size(); i++)
            dof_u[i] = mat(_dof_indices[i], _dof_indices[i]);
        }
        tag++;
      }
    }

    if (_need_dof_values_previous_nl)
    {
      _dof_values_previous_nl.resize(n);
      _sys.solutionPreviousNewton()->get(_dof_indices, &_dof_values_previous_nl[0]);
      _nodal_value_previous_nl = _dof_values_previous_nl[0];
    }

    if (_subproblem.isTransient())
    {
      _dof_values_old.resize(n);
      _dof_values_older.resize(n);
      _sys.solutionOld().get(_dof_indices, &_dof_values_old[0]);
      _sys.solutionOlder().get(_dof_indices, &_dof_values_older[0]);
      _nodal_value_old = _dof_values_old[0];
      _nodal_value_older = _dof_values_older[0];

      _dof_values_dot.resize(n);
      _dof_du_dot_du.resize(n);
      _dof_values_dot[0] = _sys.solutionUDot()(_dof_indices[0]);
      _dof_du_dot_du[0] = _sys.duDotDu();
      _nodal_value_dot = _dof_values_dot[0];
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
      _dof_du_dot_du.resize(0);
    }
  }
}

template <>
void
MooseVariableFE<RealVectorValue>::computeNodalValues()
{
  if (_has_dofs)
  {
    const std::size_t n = _dof_indices.size();
    mooseAssert(n, "There must be a non-zero number of degrees of freedom");
    _dof_values.resize(n);
    _sys.currentSolution()->get(_dof_indices, &_dof_values[0]);
    for (size_t i = 0; i < n; ++i)
      _nodal_value(i) = _dof_values[i];

    if (_need_dof_values_previous_nl)
    {
      _dof_values_previous_nl.resize(n);
      _sys.solutionPreviousNewton()->get(_dof_indices, &_dof_values_previous_nl[0]);
      _nodal_value_previous_nl = _dof_values_previous_nl[0];
      for (size_t i = 0; i < n; ++i)
        _nodal_value_previous_nl(i) = _dof_values_previous_nl[i];
    }

    if (_subproblem.isTransient())
    {
      _dof_values_old.resize(n);
      _dof_values_older.resize(n);
      _sys.solutionOld().get(_dof_indices, &_dof_values_old[0]);
      _sys.solutionOlder().get(_dof_indices, &_dof_values_older[0]);
      for (size_t i = 0; i < n; ++i)
      {
        _nodal_value_old(i) = _dof_values_old[i];
        _nodal_value_older(i) = _dof_values_older[i];
      }
      _dof_values_dot.resize(n);
      _dof_du_dot_du.resize(n);
      for (size_t i = 0; i < n; ++i)
      {
        _dof_values_dot[i] = _sys.solutionUDot()(_dof_indices[i]);
        _nodal_value_dot(i) = _dof_values_dot[i];
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
      _dof_du_dot_du.resize(0);
    }
  }
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeNodalNeighborValues()
{
  if (_neighbor_has_dofs)
  {
    const unsigned int n = _dof_indices_neighbor.size();
    mooseAssert(n, "There must be a non-zero number of degrees of freedom");
    _dof_values_neighbor.resize(n);
    _sys.currentSolution()->get(_dof_indices_neighbor, &_dof_values_neighbor[0]);

    if (_subproblem.isTransient())
    {
      _dof_values_old_neighbor.resize(n);
      _dof_values_older_neighbor.resize(n);
      _sys.solutionOld().get(_dof_indices_neighbor, &_dof_values_old_neighbor[0]);
      _sys.solutionOlder().get(_dof_indices_neighbor, &_dof_values_older_neighbor[0]);

      _dof_values_dot_neighbor.resize(n);
      _dof_du_dot_du_neighbor.resize(n);
      for (unsigned int i = 0; i < n; i++)
      {
        _dof_values_dot_neighbor[i] = _sys.solutionUDot()(_dof_indices_neighbor[i]);
        _dof_du_dot_du_neighbor[i] = _sys.duDotDu();
      }
    }
  }
  else
  {
    _dof_values_neighbor.resize(0);
    if (_subproblem.isTransient())
    {
      _dof_values_old_neighbor.resize(0);
      _dof_values_older_neighbor.resize(0);
      _dof_values_dot_neighbor.resize(0);
      _dof_du_dot_du_neighbor.resize(0);
    }
  }
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::isVector() const
{
  return std::is_same<OutputType, RealVectorValue>::value;
}

template class MooseVariableFE<Real>;
template class MooseVariableFE<RealVectorValue>;
