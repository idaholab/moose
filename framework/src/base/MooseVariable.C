/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MooseVariable.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "MooseMesh.h"

// libMesh
#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/quadrature.h"
#include "libmesh/dense_vector.h"

MooseVariable::MooseVariable(unsigned int var_num,
                             const FEType & fe_type,
                             SystemBase & sys,
                             Assembly & assembly,
                             Moose::VarKindType var_kind)
  : MooseVariableBase(var_num, fe_type, sys, assembly, var_kind),

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

    _need_nodal_u(false),
    _need_nodal_u_old(false),
    _need_nodal_u_older(false),
    _need_nodal_u_previous_nl(false),
    _need_nodal_u_dot(false),

    _need_nodal_u_neighbor(false),
    _need_nodal_u_old_neighbor(false),
    _need_nodal_u_older_neighbor(false),
    _need_nodal_u_previous_nl_neighbor(false),
    _need_nodal_u_dot_neighbor(false),

    _phi(_assembly.fePhi(_fe_type)),
    _grad_phi(_assembly.feGradPhi(_fe_type)),

    _phi_face(_assembly.fePhiFace(_fe_type)),
    _grad_phi_face(_assembly.feGradPhiFace(_fe_type)),

    _phi_neighbor(_assembly.fePhiNeighbor(_fe_type)),
    _grad_phi_neighbor(_assembly.feGradPhiNeighbor(_fe_type)),

    _phi_face_neighbor(_assembly.fePhiFaceNeighbor(_fe_type)),
    _grad_phi_face_neighbor(_assembly.feGradPhiFaceNeighbor(_fe_type)),

    _normals(_assembly.normals()),

    _is_defined(false),
    _has_nodal_value(false),
    _has_nodal_value_neighbor(false),

    _node(_assembly.node()),
    _is_defined_neighbor(false),
    _node_neighbor(_assembly.nodeNeighbor())
{
  _assembly.buildFE(feType());

  // FIXME: continuity of FE type seems equivalent with the definition of nodal variables.
  //        Continuity does not depend on the FE dimension, so we just pass in a valid dimension.
  _is_nodal = _assembly.getFE(feType(), _sys.mesh().dimension())->get_continuity() != DISCONTINUOUS;
}

MooseVariable::~MooseVariable()
{
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

  _second_u.release();
  _second_u_bak.release();
  _second_u_old.release();
  _second_u_old_bak.release();
  _second_u_older.release();
  _second_u_older_bak.release();
  _second_u_previous_nl.release();

  _u_dot.release();
  _u_dot_bak.release();
  _u_dot_neighbor.release();
  _u_dot_bak_neighbor.release();

  _du_dot_du.release();
  _du_dot_du_bak.release();
  _du_dot_du_neighbor.release();
  _du_dot_du_bak_neighbor.release();

  _nodal_u.release();
  _nodal_u_old.release();
  _nodal_u_older.release();
  _nodal_u_previous_nl.release();
  _nodal_u_dot.release();
  _nodal_du_dot_du.release();

  _nodal_u_neighbor.release();
  _nodal_u_old_neighbor.release();
  _nodal_u_older_neighbor.release();
  _nodal_u_previous_nl_neighbor.release();
  _nodal_u_dot_neighbor.release();
  _nodal_du_dot_du_neighbor.release();

  _increment.release();

  _u_neighbor.release();
  _u_old_neighbor.release();
  _u_older_neighbor.release();
  _u_previous_nl_neighbor.release();

  _grad_u_neighbor.release();
  _grad_u_old_neighbor.release();
  _grad_u_older_neighbor.release();
  _grad_u_previous_nl_neighbor.release();

  _second_u_neighbor.release();
  _second_u_old_neighbor.release();
  _second_u_older_neighbor.release();
  _second_u_previous_nl_neighbor.release();
}

const std::set<SubdomainID> &
MooseVariable::activeSubdomains()
{
  return _sys.system().variable(_var_num).active_subdomains();
}

bool
MooseVariable::activeOnSubdomain(SubdomainID subdomain) const
{
  return _sys.system().variable(_var_num).active_on_subdomain(subdomain);
}

bool
MooseVariable::isNodal() const
{
  return _is_nodal;
}

void
MooseVariable::clearDofIndices()
{
  _dof_indices.clear();
}

void
MooseVariable::prepare()
{
  _dof_map.dof_indices(_elem, _dof_indices, _var_num);
  _has_nodal_value = false;
  _has_nodal_value_neighbor = false;

  // FIXME: remove this when the Richard's module is migrated to use the new NodalCoupleable
  // interface.
  if (_dof_indices.size() > 0)
    _is_defined = true;
  else
    _is_defined = false;
}

void
MooseVariable::prepareNeighbor()
{
  _dof_map.dof_indices(_neighbor, _dof_indices_neighbor, _var_num);
  _has_nodal_value = false;
  _has_nodal_value_neighbor = false;
}

void
MooseVariable::prepareAux()
{
  _has_nodal_value = false;
  _has_nodal_value_neighbor = false;
}

void
MooseVariable::prepareIC()
{
  _dof_map.dof_indices(_elem, _dof_indices, _var_num);
  _nodal_u.resize(_dof_indices.size());

  unsigned int nqp = _qrule->n_points();
  _u.resize(nqp);
}

void
MooseVariable::reinitNode()
{
  if (_node->n_dofs(_sys.number(), _var_num) > 0)
  {
    _nodal_dof_index = _node->dof_number(_sys.number(), _var_num, 0);
    _dof_indices.resize(1);
    _dof_indices[0] = _nodal_dof_index;
    _is_defined = true;
  }
  else
  {
    _dof_indices.clear(); // Clear these so Assembly doesn't think there's dofs here
    _is_defined = false;
  }
}

void
MooseVariable::reinitNodeNeighbor()
{
  if (_node_neighbor->n_dofs(_sys.number(), _var_num) > 0)
  {
    _nodal_dof_index_neighbor = _node_neighbor->dof_number(_sys.number(), _var_num, 0);
    _dof_indices_neighbor.resize(1);
    _dof_indices_neighbor[0] = _nodal_dof_index_neighbor;
    _is_defined_neighbor = true;
  }
  else
  {
    _dof_indices_neighbor.clear(); // Clear these so Assembly doesn't think there's dofs here
    _is_defined_neighbor = false;
  }
}

void
MooseVariable::reinitAux()
{
  /* FIXME: this method is only for elemental auxiliary variables, so
   * we may want to rename it */
  _dof_map.dof_indices(_elem, _dof_indices, _var_num);
  if (_elem->n_dofs(_sys.number(), _var_num) > 0)
  {
    // FIXME: check if the following is equivalent with '_nodal_dof_index = _dof_indices[0];'?
    _nodal_dof_index = _elem->dof_number(_sys.number(), _var_num, 0);
    libmesh_assert(_dof_indices.size());
    _nodal_u.resize(_dof_indices.size());
    _sys.currentSolution()->get(_dof_indices, &_nodal_u[0]);

    _is_defined = true;
  }
  else
    _is_defined = false;
}

void
MooseVariable::reinitAuxNeighbor()
{
  if (_neighbor)
  {
    _dof_map.dof_indices(_neighbor, _dof_indices_neighbor, _var_num);
    if (_neighbor->n_dofs(_sys.number(), _var_num) > 0)
    {
      _nodal_dof_index_neighbor = _neighbor->dof_number(_sys.number(), _var_num, 0);

      libmesh_assert(_dof_indices_neighbor.size());
      _nodal_u_neighbor.resize(_dof_indices_neighbor.size());
      _sys.currentSolution()->get(_dof_indices_neighbor, &_nodal_u_neighbor[0]);

      _is_defined_neighbor = true;
    }
    else
      _is_defined_neighbor = false;
  }
  else
    _is_defined_neighbor = false;
}

void
MooseVariable::reinitNodes(const std::vector<dof_id_type> & nodes)
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
    _is_defined = true;
  else
    _is_defined = false;
}

void
MooseVariable::reinitNodesNeighbor(const std::vector<dof_id_type> & nodes)
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
    _is_defined_neighbor = true;
  else
    _is_defined_neighbor = false;
}

void
MooseVariable::getDofIndices(const Elem * elem, std::vector<dof_id_type> & dof_indices)
{
  _dof_map.dof_indices(elem, dof_indices, _var_num);
}

void
MooseVariable::insert(NumericVector<Number> & residual)
{
  if (_has_nodal_value)
    residual.insert(&_nodal_u[0], _dof_indices);

  if (_has_nodal_value_neighbor)
    residual.insert(&_nodal_u_neighbor[0], _dof_indices_neighbor);
}

void
MooseVariable::add(NumericVector<Number> & residual)
{
  if (_has_nodal_value)
    residual.add_vector(&_nodal_u[0], _dof_indices);

  if (_has_nodal_value_neighbor)
    residual.add_vector(&_nodal_u_neighbor[0], _dof_indices_neighbor);
}

const VariablePhiValue &
MooseVariable::phi()
{
  return _phi;
}

const VariablePhiGradient &
MooseVariable::gradPhi()
{
  return _grad_phi;
}

const VariablePhiSecond &
MooseVariable::secondPhi()
{
  _second_phi = &_assembly.feSecondPhi(_fe_type);
  return *_second_phi;
}

const VariablePhiValue &
MooseVariable::phiFace()
{
  return _phi_face;
}

const VariablePhiGradient &
MooseVariable::gradPhiFace()
{
  return _grad_phi_face;
}

const VariablePhiSecond &
MooseVariable::secondPhiFace()
{
  _second_phi_face = &_assembly.feSecondPhiFace(_fe_type);
  return *_second_phi_face;
}

const VariablePhiValue &
MooseVariable::phiNeighbor()
{
  return _phi_neighbor;
}

const VariablePhiGradient &
MooseVariable::gradPhiNeighbor()
{
  return _grad_phi_neighbor;
}

const VariablePhiSecond &
MooseVariable::secondPhiNeighbor()
{
  _second_phi_neighbor = &_assembly.feSecondPhiNeighbor(_fe_type);
  return *_second_phi_neighbor;
}

const VariablePhiValue &
MooseVariable::phiFaceNeighbor()
{
  return _phi_face_neighbor;
}

const VariablePhiGradient &
MooseVariable::gradPhiFaceNeighbor()
{
  return _grad_phi_face_neighbor;
}

const VariablePhiSecond &
MooseVariable::secondPhiFaceNeighbor()
{
  _second_phi_face_neighbor = &_assembly.feSecondPhiFaceNeighbor(_fe_type);
  return *_second_phi_face_neighbor;
}

const VariableValue &
MooseVariable::nodalValue()
{
  if (isNodal())
  {
    _need_nodal_u = true;
    return _nodal_u;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const VariableValue &
MooseVariable::nodalValueOld()
{
  if (isNodal())
  {
    _need_nodal_u_old = true;
    return _nodal_u_old;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const VariableValue &
MooseVariable::nodalValueOlder()
{
  if (isNodal())
  {
    _need_nodal_u_older = true;
    return _nodal_u_older;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const VariableValue &
MooseVariable::nodalValuePreviousNL()
{
  if (isNodal())
  {
    _need_nodal_u_previous_nl = true;
    return _nodal_u_previous_nl;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const VariableValue &
MooseVariable::nodalValueDot()
{
  if (isNodal())
  {
    _need_nodal_u_dot = true;
    return _nodal_u_dot;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const VariableValue &
MooseVariable::nodalValueNeighbor()
{
  if (isNodal())
  {
    _need_nodal_u_neighbor = true;
    return _nodal_u_neighbor;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const VariableValue &
MooseVariable::nodalValueOldNeighbor()
{
  if (isNodal())
  {
    _need_nodal_u_old_neighbor = true;
    return _nodal_u_old_neighbor;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const VariableValue &
MooseVariable::nodalValueOlderNeighbor()
{
  if (isNodal())
  {
    _need_nodal_u_older_neighbor = true;
    return _nodal_u_older_neighbor;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const VariableValue &
MooseVariable::nodalValuePreviousNLNeighbor()
{
  if (isNodal())
  {
    _need_nodal_u_previous_nl_neighbor = true;
    return _nodal_u_previous_nl_neighbor;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

const VariableValue &
MooseVariable::nodalValueDotNeighbor()
{
  if (isNodal())
  {
    _need_nodal_u_dot_neighbor = true;
    return _nodal_u_dot_neighbor;
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               name(),
               "' is not nodal.");
}

// FIXME: this and computeElemeValues() could be refactored to reuse most of
//        the common code, instead of duplicating it.
void
MooseVariable::computePerturbedElemValues(unsigned int perturbation_idx,
                                          Real perturbation_scale,
                                          Real & perturbation)
{

  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _qrule->n_points();

  _u_bak = _u;
  _grad_u_bak = _grad_u;
  _u.resize(nqp);
  _grad_u.resize(nqp);

  if (_need_second)
    _second_u_bak = _second_u;
  _second_u.resize(nqp);

  if (is_transient)
  {
    _u_dot_bak = _u_dot;
    _u_dot.resize(nqp);
    _du_dot_du_bak = _du_dot_du;
    _du_dot_du.resize(nqp);

    if (_need_u_old)
      _u_old_bak = _u_old;
    _u_old.resize(nqp);

    if (_need_u_older)
      _u_older_bak = _u_older;
    _u_older.resize(nqp);

    if (_need_grad_old)
      _grad_u_old_bak = _grad_u_old;
    _grad_u_old.resize(nqp);

    if (_need_grad_older)
      _grad_u_older_bak = _grad_u_older;
    _grad_u_older.resize(nqp);

    if (_need_second_old)
      _second_u_old_bak = _second_u_old;
    _second_u_old.resize(nqp);

    if (_need_second_older)
      _second_u_older_bak = _second_u_older;
    _second_u_older.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u[i] = 0;
    _grad_u[i] = 0;

    if (_need_second)
      _second_u[i] = 0;

    if (is_transient)
    {
      _u_dot[i] = 0;
      _du_dot_du[i] = 0;

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
    }
  }

  unsigned int num_dofs = _dof_indices.size();

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old = _sys.solutionOld();
  const NumericVector<Real> & solution_older = _sys.solutionOlder();
  const NumericVector<Real> & u_dot = _sys.solutionUDot();
  const Real & du_dot_du = _sys.duDotDu();

  dof_id_type idx = 0;
  Real soln_local = 0;
  Real soln_old_local = 0;
  Real soln_older_local = 0;
  Real u_dot_local = 0;

  Real phi_local = 0;
  const RealGradient * dphi_qp = NULL;
  const RealTensor * d2phi_local = NULL;

  RealGradient * grad_u_qp = NULL;

  RealGradient * grad_u_old_qp = NULL;
  RealGradient * grad_u_older_qp = NULL;

  RealTensor * second_u_qp = NULL;

  RealTensor * second_u_old_qp = NULL;
  RealTensor * second_u_older_qp = NULL;

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    idx = _dof_indices[i];
    soln_local = current_solution(idx);
    if (i == perturbation_idx)
    {
      // Compute the size of the perturbation.
      // For the PETSc DS differencing method we use the magnitude of the variable at the "node"
      // to determine the differencing parameters.  The WP method could use the element L2 norm of
      // the variable  instead.
      perturbation = soln_local;
      // HACK: the use of fabs() and < assume Real is double or similar. Otherwise need to use
      // PetscAbsScalar, PetscRealPart, etc.
      if (fabs(perturbation) < 1.0e-16)
        perturbation = (perturbation < 0. ? -1.0 : 1.0) * 0.1;
      perturbation *= perturbation_scale;
      soln_local += perturbation;
    }
    if (is_transient)
    {
      if (_need_u_old || _need_grad_old || _need_second_old)
        soln_old_local = solution_old(idx);

      if (_need_u_older || _need_grad_older || _need_second_older)
        soln_older_local = solution_older(idx);

      u_dot_local = u_dot(idx);
    }

    for (unsigned int qp = 0; qp < nqp; qp++)
    {
      phi_local = _phi[i][qp];
      dphi_qp = &_grad_phi[i][qp];

      grad_u_qp = &_grad_u[qp];

      if (is_transient)
      {
        if (_need_grad_old)
          grad_u_old_qp = &_grad_u_old[qp];

        if (_need_grad_older)
          grad_u_older_qp = &_grad_u_older[qp];
      }

      if (_need_second || _need_second_old || _need_second_older)
      {
        d2phi_local = &(*_second_phi)[i][qp];

        if (_need_second)
        {
          second_u_qp = &_second_u[qp];
          second_u_qp->add_scaled(*d2phi_local, soln_local);
        }

        if (is_transient)
        {
          if (_need_second_old)
            second_u_old_qp = &_second_u_old[qp];

          if (_need_second_older)
            second_u_older_qp = &_second_u_older[qp];
        }
      }

      _u[qp] += phi_local * soln_local;

      grad_u_qp->add_scaled(*dphi_qp, soln_local);

      if (is_transient)
      {
        _u_dot[qp] += phi_local * u_dot_local;
        _du_dot_du[qp] = du_dot_du;

        if (_need_u_old)
          _u_old[qp] += phi_local * soln_old_local;

        if (_need_u_older)
          _u_older[qp] += phi_local * soln_older_local;

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

void
MooseVariable::restoreUnperturbedElemValues()
{
  _u = _u_bak;
  _grad_u = _grad_u_bak;
  if (_need_second)
    _second_u = _second_u_bak;

  if (_subproblem.isTransient())
  {
    _u_dot = _u_dot_bak;
    _du_dot_du = _du_dot_du_bak;

    if (_need_u_old)
      _u_old = _u_old_bak;

    if (_need_u_older)
      _u_older = _u_older_bak;

    if (_need_grad_old)
      _grad_u_old = _grad_u_old_bak;

    if (_need_grad_older)
      _grad_u_older = _grad_u_older_bak;

    if (_need_second_old)
      _second_u_old = _second_u_old_bak;

    if (_need_second_older)
      _second_u_older = _second_u_older_bak;
  }
}

void
MooseVariable::computeElemValues()
{

  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _qrule->n_points();

  _u.resize(nqp);
  _grad_u.resize(nqp);

  if (_need_second)
    _second_u.resize(nqp);

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

    if (_need_second_older)
      _second_u_older.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u[i] = 0;
    _grad_u[i] = 0;

    if (_need_second)
      _second_u[i] = 0;

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
    }
  }

  unsigned int num_dofs = _dof_indices.size();

  if (_need_nodal_u)
    _nodal_u.resize(num_dofs);

  if (_need_nodal_u_previous_nl)
    _nodal_u_previous_nl.resize(num_dofs);

  if (is_transient)
  {
    if (_need_nodal_u_old)
      _nodal_u_old.resize(num_dofs);
    if (_need_nodal_u_older)
      _nodal_u_older.resize(num_dofs);
    if (_need_nodal_u_dot)
      _nodal_u_dot.resize(num_dofs);
  }

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

  Real phi_local = 0;
  const RealGradient * dphi_qp = NULL;
  const RealTensor * d2phi_local = NULL;

  RealGradient * grad_u_qp = NULL;

  RealGradient * grad_u_old_qp = NULL;
  RealGradient * grad_u_older_qp = NULL;
  RealGradient * grad_u_previous_nl_qp = NULL;

  RealTensor * second_u_qp = NULL;

  RealTensor * second_u_old_qp = NULL;
  RealTensor * second_u_older_qp = NULL;
  RealTensor * second_u_previous_nl_qp = NULL;

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    idx = _dof_indices[i];
    soln_local = current_solution(idx);

    if (_need_nodal_u)
      _nodal_u[i] = soln_local;

    if (_need_u_previous_nl || _need_grad_previous_nl || _need_second_previous_nl ||
        _need_nodal_u_previous_nl)
      soln_previous_nl_local = (*solution_prev_nl)(idx);

    if (_need_nodal_u_previous_nl)
      _nodal_u_previous_nl[i] = soln_previous_nl_local;

    if (is_transient)
    {
      if (_need_u_old || _need_grad_old || _need_second_old || _need_nodal_u_old)
        soln_old_local = solution_old(idx);

      if (_need_u_older || _need_grad_older || _need_second_older || _need_nodal_u_older)
        soln_older_local = solution_older(idx);

      if (_need_nodal_u_old)
        _nodal_u_old[i] = soln_old_local;
      if (_need_nodal_u_older)
        _nodal_u_older[i] = soln_older_local;

      u_dot_local = u_dot(idx);
      if (_need_nodal_u_dot)
        _nodal_u_dot[i] = u_dot_local;
    }

    for (unsigned int qp = 0; qp < nqp; qp++)
    {
      phi_local = _phi[i][qp];
      dphi_qp = &_grad_phi[i][qp];

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
        d2phi_local = &(*_second_phi)[i][qp];

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

      _u[qp] += phi_local * soln_local;

      grad_u_qp->add_scaled(*dphi_qp, soln_local);

      if (_need_u_previous_nl)
        _u_previous_nl[qp] += phi_local * soln_previous_nl_local;
      if (_need_grad_previous_nl)
        grad_u_previous_nl_qp->add_scaled(*dphi_qp, soln_previous_nl_local);

      if (is_transient)
      {
        _u_dot[qp] += phi_local * u_dot_local;
        _du_dot_du[qp] = du_dot_du;

        if (_need_u_old)
          _u_old[qp] += phi_local * soln_old_local;

        if (_need_u_older)
          _u_older[qp] += phi_local * soln_older_local;

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

void
MooseVariable::computeElemValuesFace()
{
  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _qrule_face->n_points();
  _u.resize(nqp);
  _grad_u.resize(nqp);

  if (_need_second)
    _second_u.resize(nqp);

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

    if (_need_second_older)
      _second_u_older.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u[i] = 0;
    _grad_u[i] = 0;

    if (_need_second)
      _second_u[i] = 0;

    if (_need_u_previous_nl)
      _u_previous_nl[i] = 0;

    if (_need_grad_previous_nl)
      _grad_u_previous_nl[i] = 0;

    if (_need_second_previous_nl)
      _second_u_previous_nl[i] = 0;

    if (_subproblem.isTransient())
    {
      _u_dot[i] = 0;
      _du_dot_du[i] = 0;

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
    }
  }

  unsigned int num_dofs = _dof_indices.size();

  if (_need_nodal_u)
    _nodal_u.resize(num_dofs);
  if (_need_nodal_u_previous_nl)
    _nodal_u_previous_nl.resize(num_dofs);
  if (is_transient)
  {
    if (_need_nodal_u_old)
      _nodal_u_old.resize(num_dofs);
    if (_need_nodal_u_older)
      _nodal_u_older.resize(num_dofs);
    if (_need_nodal_u_dot)
      _nodal_u_dot.resize(num_dofs);
  }

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old = _sys.solutionOld();
  const NumericVector<Real> & solution_older = _sys.solutionOlder();
  const NumericVector<Real> * solution_prev_nl = _sys.solutionPreviousNewton();
  const NumericVector<Real> & u_dot = _sys.solutionUDot();
  const Real & du_dot_du = _sys.duDotDu();

  dof_id_type idx;
  Real soln_local;
  Real soln_old_local = 0;
  Real soln_older_local = 0;
  Real soln_previous_nl_local = 0;
  Real u_dot_local = 0;

  Real phi_local;
  RealGradient dphi_local;
  RealTensor d2phi_local;

  for (unsigned int i = 0; i < num_dofs; ++i)
  {
    idx = _dof_indices[i];
    soln_local = current_solution(idx);

    if (_need_nodal_u)
      _nodal_u[i] = soln_local;

    if (_need_u_previous_nl || _need_grad_previous_nl || _need_second_previous_nl ||
        _need_nodal_u_previous_nl)
      soln_previous_nl_local = (*solution_prev_nl)(idx);
    if (_need_nodal_u_previous_nl)
      _nodal_u_previous_nl[i] = soln_previous_nl_local;

    if (is_transient)
    {
      if (_need_u_old || _need_grad_old || _need_second_old || _need_nodal_u_old)
        soln_old_local = solution_old(idx);

      if (_need_u_older || _need_grad_older || _need_second_older || _need_nodal_u_older)
        soln_older_local = solution_older(idx);

      if (_need_nodal_u_old)
        _nodal_u_old[i] = soln_old_local;
      if (_need_nodal_u_older)
        _nodal_u_older[i] = soln_older_local;

      u_dot_local = u_dot(idx);
      if (_need_nodal_u_dot)
        _nodal_u_dot[i] = u_dot_local;
    }

    for (unsigned int qp = 0; qp < nqp; ++qp)
    {
      phi_local = _phi_face[i][qp];
      dphi_local = _grad_phi_face[i][qp];

      if (_need_second || _need_second_old || _need_second_older || _need_second_previous_nl)
        d2phi_local = (*_second_phi_face)[i][qp];

      _u[qp] += phi_local * soln_local;
      _grad_u[qp] += dphi_local * soln_local;

      if (_need_second)
        _second_u[qp] += d2phi_local * soln_local;

      if (_need_u_previous_nl)
        _u_previous_nl[qp] += phi_local * soln_previous_nl_local;

      if (_need_grad_previous_nl)
        _grad_u_previous_nl[qp] += dphi_local * soln_previous_nl_local;

      if (_need_second_previous_nl)
        _second_u_previous_nl[qp] += d2phi_local * soln_previous_nl_local;

      if (is_transient)
      {
        _u_dot[qp] += phi_local * u_dot_local;
        _du_dot_du[qp] = du_dot_du;

        if (_need_u_old)
          _u_old[qp] += phi_local * soln_old_local;

        if (_need_u_older)
          _u_older[qp] += phi_local * soln_older_local;

        if (_need_grad_old)
          _grad_u_old[qp] += dphi_local * soln_old_local;

        if (_need_grad_older)
          _grad_u_older[qp] += dphi_local * soln_older_local;

        if (_need_second_old)
          _second_u_old[qp] += d2phi_local * soln_old_local;

        if (_need_second_older)
          _second_u_older[qp] += d2phi_local * soln_older_local;
      }
    }
  }
}

void
MooseVariable::computeNeighborValuesFace()
{
  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _qrule_neighbor->n_points();

  _u_neighbor.resize(nqp);
  _grad_u_neighbor.resize(nqp);

  if (_need_second_neighbor)
    _second_u_neighbor.resize(nqp);

  if (is_transient)
  {
    _u_dot_neighbor.resize(nqp);
    _du_dot_du_neighbor.resize(nqp);

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

    if (_subproblem.isTransient())
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

  if (_need_nodal_u_neighbor)
    _nodal_u_neighbor.resize(num_dofs);
  if (is_transient)
  {
    if (_need_nodal_u_old_neighbor)
      _nodal_u_old_neighbor.resize(num_dofs);
    if (_need_nodal_u_older_neighbor)
      _nodal_u_older_neighbor.resize(num_dofs);
    if (_need_nodal_u_dot_neighbor)
      _nodal_u_dot_neighbor.resize(num_dofs);
  }

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

  Real phi_local;
  RealGradient dphi_local;
  RealTensor d2phi_local;

  for (unsigned int i = 0; i < num_dofs; ++i)
  {
    idx = _dof_indices_neighbor[i];
    soln_local = current_solution(idx);

    if (is_transient)
    {
      if (_need_u_old_neighbor || _need_grad_old_neighbor || _need_second_old_neighbor)
        soln_old_local = solution_old(idx);

      if (_need_u_older_neighbor || _need_grad_older_neighbor || _need_second_older_neighbor)
        soln_older_local = solution_older(idx);

      if (_need_nodal_u_old_neighbor)
        _nodal_u_old_neighbor[i] = soln_old_local;
      if (_need_nodal_u_older_neighbor)
        _nodal_u_older_neighbor[i] = soln_older_local;

      u_dot_local = u_dot(idx);
      if (_need_nodal_u_dot_neighbor)
        _nodal_u_dot_neighbor[i] = u_dot_local;
    }

    for (unsigned int qp = 0; qp < nqp; ++qp)
    {
      phi_local = _phi_face_neighbor[i][qp];
      dphi_local = _grad_phi_face_neighbor[i][qp];

      if (_need_second_neighbor || _need_second_old_neighbor || _need_second_older_neighbor)
        d2phi_local = (*_second_phi_face_neighbor)[i][qp];

      _u_neighbor[qp] += phi_local * soln_local;
      _grad_u_neighbor[qp] += dphi_local * soln_local;

      if (_need_second_neighbor)
        _second_u_neighbor[qp] += d2phi_local * soln_local;

      if (is_transient)
      {
        _u_dot_neighbor[qp] += phi_local * u_dot_local;
        _du_dot_du_neighbor[qp] = du_dot_du;

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

void
MooseVariable::computeNeighborValues()
{
  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _qrule_neighbor->n_points();

  _u_neighbor.resize(nqp);
  _grad_u_neighbor.resize(nqp);

  if (_need_second_neighbor)
    _second_u_neighbor.resize(nqp);

  if (is_transient)
  {
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

    if (_subproblem.isTransient())
    {
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

  if (_need_nodal_u_neighbor)
    _nodal_u_neighbor.resize(num_dofs);
  if (is_transient)
  {
    if (_need_nodal_u_old_neighbor)
      _nodal_u_old_neighbor.resize(num_dofs);
    if (_need_nodal_u_older_neighbor)
      _nodal_u_older_neighbor.resize(num_dofs);
    if (_need_nodal_u_dot_neighbor)
      _nodal_u_dot_neighbor.resize(num_dofs);
  }

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old = _sys.solutionOld();
  const NumericVector<Real> & solution_older = _sys.solutionOlder();
  const NumericVector<Real> & u_dot = _sys.solutionUDot();

  dof_id_type idx;
  Real soln_local;
  Real soln_old_local = 0;
  Real soln_older_local = 0;
  Real u_dot_local = 0;

  Real phi_local;
  RealGradient dphi_local;
  RealTensor d2phi_local;

  for (unsigned int i = 0; i < num_dofs; ++i)
  {
    idx = _dof_indices_neighbor[i];
    soln_local = current_solution(idx);

    if (is_transient)
    {
      if (_need_u_old_neighbor)
        soln_old_local = solution_old(idx);

      if (_need_u_older_neighbor)
        soln_older_local = solution_older(idx);

      if (_need_nodal_u_old_neighbor)
        _nodal_u_old_neighbor[i] = soln_old_local;
      if (_need_nodal_u_older_neighbor)
        _nodal_u_older_neighbor[i] = soln_older_local;

      u_dot_local = u_dot(idx);
      if (_need_nodal_u_dot_neighbor)
        _nodal_u_dot_neighbor[i] = u_dot_local;
    }

    for (unsigned int qp = 0; qp < nqp; ++qp)
    {
      phi_local = _phi_neighbor[i][qp];
      dphi_local = _grad_phi_neighbor[i][qp];

      if (_need_second_neighbor || _need_second_old_neighbor || _need_second_older_neighbor)
        d2phi_local = (*_second_phi_neighbor)[i][qp];

      _u_neighbor[qp] += phi_local * soln_local;
      _grad_u_neighbor[qp] += dphi_local * soln_local;

      if (_need_second_neighbor)
        _second_u_neighbor[qp] += d2phi_local * soln_local;

      if (is_transient)
      {
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

void
MooseVariable::computeNodalValues()
{
  if (_is_defined)
  {
    const unsigned int n = _dof_indices.size();
    mooseAssert(n, "Defined but empty?");
    _nodal_u.resize(n);
    _sys.currentSolution()->get(_dof_indices, &_nodal_u[0]);

    if (_need_nodal_u_previous_nl)
    {
      _nodal_u_previous_nl.resize(n);
      _sys.solutionPreviousNewton()->get(_dof_indices, &_nodal_u_previous_nl[0]);
    }

    if (_subproblem.isTransient())
    {
      _nodal_u_old.resize(n);
      _nodal_u_older.resize(n);
      _sys.solutionOld().get(_dof_indices, &_nodal_u_old[0]);
      _sys.solutionOlder().get(_dof_indices, &_nodal_u_older[0]);

      _nodal_u_dot.resize(n);
      _nodal_du_dot_du.resize(n);
      for (unsigned int i = 0; i < n; i++)
      {
        _nodal_u_dot[i] = _sys.solutionUDot()(_dof_indices[i]);
        _nodal_du_dot_du[i] = _sys.duDotDu();
      }
    }
  }
  else
  {
    _nodal_u.resize(0);
    if (_need_nodal_u_previous_nl)
      _nodal_u_previous_nl.resize(0);
    if (_subproblem.isTransient())
    {
      _nodal_u_old.resize(0);
      _nodal_u_older.resize(0);
      _nodal_u_dot.resize(0);
      _nodal_du_dot_du.resize(0);
    }
  }
}

void
MooseVariable::computeNodalNeighborValues()
{
  if (_is_defined_neighbor)
  {
    const unsigned int n = _dof_indices_neighbor.size();
    mooseAssert(n, "Defined but empty?");
    _nodal_u_neighbor.resize(n);
    _sys.currentSolution()->get(_dof_indices_neighbor, &_nodal_u_neighbor[0]);

    if (_subproblem.isTransient())
    {
      _nodal_u_old_neighbor.resize(n);
      _nodal_u_older_neighbor.resize(n);
      _sys.solutionOld().get(_dof_indices_neighbor, &_nodal_u_old_neighbor[0]);
      _sys.solutionOlder().get(_dof_indices_neighbor, &_nodal_u_older_neighbor[0]);

      _nodal_u_dot_neighbor.resize(n);
      _nodal_du_dot_du_neighbor.resize(n);
      for (unsigned int i = 0; i < n; i++)
      {
        _nodal_u_dot_neighbor[i] = _sys.solutionUDot()(_dof_indices_neighbor[i]);
        _nodal_du_dot_du_neighbor[i] = _sys.duDotDu();
      }
    }
  }
  else
  {
    _nodal_u_neighbor.resize(0);
    if (_subproblem.isTransient())
    {
      _nodal_u_old_neighbor.resize(0);
      _nodal_u_older_neighbor.resize(0);
      _nodal_u_dot_neighbor.resize(0);
      _nodal_du_dot_du_neighbor.resize(0);
    }
  }
}

void
MooseVariable::setNodalValue(const DenseVector<Number> & values)
{
  for (unsigned int i = 0; i < values.size(); i++)
    _nodal_u[i] = values(i);

  _has_nodal_value = true;

  for (unsigned int qp = 0; qp < _u.size(); qp++)
  {
    _u[qp] = 0;
    for (unsigned int i = 0; i < _nodal_u.size(); i++)
      _u[qp] += _phi[i][qp] * _nodal_u[i];
  }
}

void
MooseVariable::setNodalValueNeighbor(const DenseVector<Number> & values)
{
  for (unsigned int i = 0; i < values.size(); i++)
    _nodal_u_neighbor[i] = values(i);
  _has_nodal_value_neighbor = true;

  if (isNodal())
    mooseError("Variable " + name() + " has to be nodal!");
  else
  {
    for (unsigned int qp = 0; qp < _u_neighbor.size(); qp++)
    {
      _u_neighbor[qp] = 0;
      for (unsigned int i = 0; i < _nodal_u_neighbor.size(); i++)
        _u_neighbor[qp] = _phi_face_neighbor[i][qp] * _nodal_u_neighbor[i];
    }
  }
}

void
MooseVariable::setNodalValue(Number value, unsigned int idx /* = 0*/)
{
  _nodal_u[idx] = value; // update variable nodal value
  _has_nodal_value = true;

  // Update the qp values as well
  for (unsigned int qp = 0; qp < _u.size(); qp++)
    _u[qp] = value;
}

void
MooseVariable::setNodalValueNeighbor(Number value)
{
  _nodal_u_neighbor[0] = value; // update variable nodal value
  _has_nodal_value_neighbor = true;

  if (!isNodal()) // If this is an elemental variable, then update the qp values as well
  {
    for (unsigned int qp = 0; qp < _u_neighbor.size(); qp++)
      _u_neighbor[qp] = value;
  }
}

void
MooseVariable::computeIncrementAtQps(const NumericVector<Number> & increment_vec)
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

void
MooseVariable::computeIncrementAtNode(const NumericVector<Number> & increment_vec)
{
  if (!isNodal())
    mooseError("computeIncrementAtNode can only be called for nodal variables");

  _increment.resize(1);

  // Compute the increment for the current DOF
  _increment[0] = increment_vec(_dof_indices[0]);
}

Number
MooseVariable::getNodalValue(const Node & node)
{
  mooseAssert(_subproblem.mesh().isSemiLocal(const_cast<Node *>(&node)), "Node is not Semilocal");

  // Make sure that the node has DOFs
  /* Note, this is a reproduction of an assert within libMesh::Node::dof_number, this is done to
   * produce a
   * better error (see misc/check_error.node_value_off_block) */
  mooseAssert(node.n_dofs(_sys.number(), _var_num) > 0,
              "Node " << node.id() << " does not contain any dofs for the "
                      << _sys.system().variable_name(_var_num)
                      << " variable");

  dof_id_type dof = node.dof_number(_sys.number(), _var_num, 0);

  return (*_sys.currentSolution())(dof);
}

Number
MooseVariable::getNodalValueOld(const Node & node)
{
  mooseAssert(_subproblem.mesh().isSemiLocal(const_cast<Node *>(&node)), "Node is not Semilocal");

  // Make sure that the node has DOFs
  /* Note, this is a reproduction of an assert within libMesh::Node::dof_number, this is done to
   * produce a
   * better error (see misc/check_error.node_value_off_block) */
  mooseAssert(node.n_dofs(_sys.number(), _var_num) > 0,
              "Node " << node.id() << " does not contain any dofs for the "
                      << _sys.system().variable_name(_var_num)
                      << " variable");

  dof_id_type dof = node.dof_number(_sys.number(), _var_num, 0);
  return _sys.solutionOld()(dof);
}

Number
MooseVariable::getNodalValueOlder(const Node & node)
{
  mooseAssert(_subproblem.mesh().isSemiLocal(const_cast<Node *>(&node)), "Node is not Semilocal");

  // Make sure that the node has DOFs
  /* Note, this is a reproduction of an assert within libMesh::Node::dof_number, this is done to
   * produce a
   * better error (see misc/check_error.node_value_off_block) */
  mooseAssert(node.n_dofs(_sys.number(), _var_num) > 0,
              "Node " << node.id() << " does not contain any dofs for the "
                      << _sys.system().variable_name(_var_num)
                      << " variable");

  dof_id_type dof = node.dof_number(_sys.number(), _var_num, 0);
  return _sys.solutionOlder()(dof);
}

Real
MooseVariable::getValue(const Elem * elem, const std::vector<std::vector<Real>> & phi) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  Real value = 0;
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

RealGradient
MooseVariable::getGradient(const Elem * elem,
                           const std::vector<std::vector<RealGradient>> & grad_phi) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  RealGradient value;
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

Real
MooseVariable::getElementalValue(const Elem * elem, unsigned int idx) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  return (*_sys.currentSolution())(dof_indices[idx]);
}
