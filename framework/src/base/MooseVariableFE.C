//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableField.h"

MooseVariableFE::MooseVariableFE(unsigned int var_num,
                                 const FEType & fe_type,
                                 SystemBase & sys,
                                 Moose::VarKindType var_kind,
                                 Assembly & assembly)
  : MooseVariableBase(var_num, fe_type, sys, var_kind),
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
    _need_dof_u(false),
    _need_dof_u_old(false),
    _need_dof_u_older(false),
    _need_dof_u_previous_nl(false),
    _need_dof_u_dot(false),
    _need_dof_du_dot_du(false),
    _need_dof_u_neighbor(false),
    _need_dof_u_old_neighbor(false),
    _need_dof_u_older_neighbor(false),
    _need_dof_u_previous_nl_neighbor(false),
    _need_dof_u_dot_neighbor(false),
    _need_dof_du_dot_du_neighbor(false),
    _normals(_assembly.normals()),
    _is_nodal(true),
    _has_dofs(false),
    _neighbor_has_dofs(false),
    _has_nodal_value(false),
    _has_nodal_value_neighbor(false),
    _node(_assembly.node()),
    _node_neighbor(_assembly.nodeNeighbor())
{
}

MooseVariableFE::~MooseVariableFE()
{
  _dof_u.release();
  _dof_u_old.release();
  _dof_u_older.release();
  _dof_u_previous_nl.release();
  _dof_u_dot.release();
  _dof_du_dot_du.release();

  _dof_u_neighbor.release();
  _dof_u_old_neighbor.release();
  _dof_u_older_neighbor.release();
  _dof_u_previous_nl_neighbor.release();
  _dof_u_dot_neighbor.release();
  _dof_du_dot_du_neighbor.release();
}

const std::set<SubdomainID> &
MooseVariableFE::activeSubdomains() const
{
  return _sys.system().variable(_var_num).active_subdomains();
}

bool
MooseVariableFE::activeOnSubdomain(SubdomainID subdomain) const
{
  return _sys.system().variable(_var_num).active_on_subdomain(subdomain);
}

void
MooseVariableFE::clearDofIndices()
{
  _dof_indices.clear();
}

void
MooseVariableFE::prepare()
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

void
MooseVariableFE::prepareNeighbor()
{
  _dof_map.dof_indices(_neighbor, _dof_indices_neighbor, _var_num);
  _has_nodal_value = false;
  _has_nodal_value_neighbor = false;
}

void
MooseVariableFE::prepareAux()
{
  _has_nodal_value = false;
  _has_nodal_value_neighbor = false;
}

void
MooseVariableFE::reinitNode()
{
  if (_node->n_dofs(_sys.number(), _var_num) > 0)
  {
    _nodal_dof_index = _node->dof_number(_sys.number(), _var_num, 0);
    _dof_indices.resize(1);
    _dof_indices[0] = _nodal_dof_index;
    _has_dofs = true;
  }
  else
  {
    _dof_indices.clear(); // Clear these so Assembly doesn't think there's dofs here
    _has_dofs = false;
  }
}

void
MooseVariableFE::reinitNodeNeighbor()
{
  if (_node_neighbor->n_dofs(_sys.number(), _var_num) > 0)
  {
    _nodal_dof_index_neighbor = _node_neighbor->dof_number(_sys.number(), _var_num, 0);
    _dof_indices_neighbor.resize(1);
    _dof_indices_neighbor[0] = _nodal_dof_index_neighbor;
    _neighbor_has_dofs = true;
  }
  else
  {
    _dof_indices_neighbor.clear(); // Clear these so Assembly doesn't think there's dofs here
    _neighbor_has_dofs = false;
  }
}

void
MooseVariableFE::reinitAux()
{
  /* FIXME: this method is only for elemental auxiliary variables, so
   * we may want to rename it */
  _dof_map.dof_indices(_elem, _dof_indices, _var_num);
  if (_elem->n_dofs(_sys.number(), _var_num) > 0)
  {
    // FIXME: check if the following is equivalent with '_nodal_dof_index = _dof_indices[0];'?
    _nodal_dof_index = _elem->dof_number(_sys.number(), _var_num, 0);
    libmesh_assert(_dof_indices.size());
    _dof_u.resize(_dof_indices.size());
    _sys.currentSolution()->get(_dof_indices, &_dof_u[0]);

    _has_dofs = true;
  }
  else
    _has_dofs = false;
}

void
MooseVariableFE::reinitAuxNeighbor()
{
  if (_neighbor)
  {
    _dof_map.dof_indices(_neighbor, _dof_indices_neighbor, _var_num);
    if (_neighbor->n_dofs(_sys.number(), _var_num) > 0)
    {
      _nodal_dof_index_neighbor = _neighbor->dof_number(_sys.number(), _var_num, 0);

      libmesh_assert(_dof_indices_neighbor.size());
      _dof_u_neighbor.resize(_dof_indices_neighbor.size());
      _sys.currentSolution()->get(_dof_indices_neighbor, &_dof_u_neighbor[0]);

      _neighbor_has_dofs = true;
    }
    else
      _neighbor_has_dofs = false;
  }
  else
    _neighbor_has_dofs = false;
}

void
MooseVariableFE::reinitNodes(const std::vector<dof_id_type> & nodes)
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

void
MooseVariableFE::reinitNodesNeighbor(const std::vector<dof_id_type> & nodes)
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

void
MooseVariableFE::getDofIndices(const Elem * elem, std::vector<dof_id_type> & dof_indices)
{
  _dof_map.dof_indices(elem, dof_indices, _var_num);
}

const MooseArray<Number> &
MooseVariableFE::nodalValue()
{
  if (isNodal())
  {
    _need_dof_u = true;
    return _dof_u;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const MooseArray<Number> &
MooseVariableFE::nodalValueOld()
{
  if (isNodal())
  {
    _need_dof_u_old = true;
    return _dof_u_old;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const MooseArray<Number> &
MooseVariableFE::nodalValueOlder()
{
  if (isNodal())
  {
    _need_dof_u_older = true;
    return _dof_u_older;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const MooseArray<Number> &
MooseVariableFE::nodalValuePreviousNL()
{
  if (isNodal())
  {
    _need_dof_u_previous_nl = true;
    return _dof_u_previous_nl;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const MooseArray<Number> &
MooseVariableFE::nodalValueDot()
{
  if (isNodal())
  {
    _need_dof_u_dot = true;
    return _dof_u_dot;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const MooseArray<Number> &
MooseVariableFE::nodalValueDuDotDu()
{
  if (isNodal())
  {
    _need_dof_du_dot_du = true;
    return _dof_du_dot_du;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const MooseArray<Number> &
MooseVariableFE::nodalValueNeighbor()
{
  if (isNodal())
  {
    _need_dof_u_neighbor = true;
    return _dof_u_neighbor;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const MooseArray<Number> &
MooseVariableFE::nodalValueOldNeighbor()
{
  if (isNodal())
  {
    _need_dof_u_old_neighbor = true;
    return _dof_u_old_neighbor;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const MooseArray<Number> &
MooseVariableFE::nodalValueOlderNeighbor()
{
  if (isNodal())
  {
    _need_dof_u_older_neighbor = true;
    return _dof_u_older_neighbor;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const MooseArray<Number> &
MooseVariableFE::nodalValuePreviousNLNeighbor()
{
  if (isNodal())
  {
    _need_dof_u_previous_nl_neighbor = true;
    return _dof_u_previous_nl_neighbor;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const MooseArray<Number> &
MooseVariableFE::nodalValueDotNeighbor()
{
  if (isNodal())
  {
    _need_dof_u_dot_neighbor = true;
    return _dof_u_dot_neighbor;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const MooseArray<Number> &
MooseVariableFE::nodalValueDuDotDuNeighbor()
{
  if (isNodal())
  {
    _need_dof_du_dot_du_neighbor = true;
    return _dof_du_dot_du_neighbor;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

void
MooseVariableFE::computeNodalValues()
{
  if (_has_dofs)
  {
    const unsigned int n = _dof_indices.size();
    mooseAssert(n, "Defined but empty?");
    _dof_u.resize(n);
    _sys.currentSolution()->get(_dof_indices, &_dof_u[0]);

    if (_need_dof_u_previous_nl)
    {
      _dof_u_previous_nl.resize(n);
      _sys.solutionPreviousNewton()->get(_dof_indices, &_dof_u_previous_nl[0]);
    }

    if (_subproblem.isTransient())
    {
      _dof_u_old.resize(n);
      _dof_u_older.resize(n);
      _sys.solutionOld().get(_dof_indices, &_dof_u_old[0]);
      _sys.solutionOlder().get(_dof_indices, &_dof_u_older[0]);

      _dof_u_dot.resize(n);
      _dof_du_dot_du.resize(n);
      for (unsigned int i = 0; i < n; i++)
      {
        _dof_u_dot[i] = _sys.solutionUDot()(_dof_indices[i]);
        _dof_du_dot_du[i] = _sys.duDotDu();
      }
    }
  }
  else
  {
    _dof_u.resize(0);
    if (_need_dof_u_previous_nl)
      _dof_u_previous_nl.resize(0);
    if (_subproblem.isTransient())
    {
      _dof_u_old.resize(0);
      _dof_u_older.resize(0);
      _dof_u_dot.resize(0);
      _dof_du_dot_du.resize(0);
    }
  }
}

void
MooseVariableFE::computeNodalNeighborValues()
{
  if (_neighbor_has_dofs)
  {
    const unsigned int n = _dof_indices_neighbor.size();
    mooseAssert(n, "Defined but empty?");
    _dof_u_neighbor.resize(n);
    _sys.currentSolution()->get(_dof_indices_neighbor, &_dof_u_neighbor[0]);

    if (_subproblem.isTransient())
    {
      _dof_u_old_neighbor.resize(n);
      _dof_u_older_neighbor.resize(n);
      _sys.solutionOld().get(_dof_indices_neighbor, &_dof_u_old_neighbor[0]);
      _sys.solutionOlder().get(_dof_indices_neighbor, &_dof_u_older_neighbor[0]);

      _dof_u_dot_neighbor.resize(n);
      _dof_du_dot_du_neighbor.resize(n);
      for (unsigned int i = 0; i < n; i++)
      {
        _dof_u_dot_neighbor[i] = _sys.solutionUDot()(_dof_indices_neighbor[i]);
        _dof_du_dot_du_neighbor[i] = _sys.duDotDu();
      }
    }
  }
  else
  {
    _dof_u_neighbor.resize(0);
    if (_subproblem.isTransient())
    {
      _dof_u_old_neighbor.resize(0);
      _dof_u_older_neighbor.resize(0);
      _dof_u_dot_neighbor.resize(0);
      _dof_du_dot_du_neighbor.resize(0);
    }
  }
}

Number
MooseVariableFE::getNodalValue(const Node & node)
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

Number
MooseVariableFE::getNodalValueOld(const Node & node)
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

Number
MooseVariableFE::getNodalValueOlder(const Node & node)
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

Number
MooseVariableFE::getElementalValue(const Elem * elem, unsigned int idx) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  return (*_sys.currentSolution())(dof_indices[idx]);
}

void
MooseVariableFE::insert(NumericVector<Number> & residual)
{
  if (_has_nodal_value)
    residual.insert(&_dof_u[0], _dof_indices);

  if (_has_nodal_value_neighbor)
    residual.insert(&_dof_u_neighbor[0], _dof_indices_neighbor);
}

void
MooseVariableFE::add(NumericVector<Number> & residual)
{
  if (_has_nodal_value)
    residual.add_vector(&_dof_u[0], _dof_indices);

  if (_has_nodal_value_neighbor)
    residual.add_vector(&_dof_u_neighbor[0], _dof_indices_neighbor);
}

const MooseArray<Number> &
MooseVariableFE::dofValue()
{
  _need_dof_u = true;
  return _dof_u;
}
