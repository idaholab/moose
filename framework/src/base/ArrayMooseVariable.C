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

#include "ArrayMooseVariable.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "NonlinearSystem.h"
#include "Assembly.h"
#include "MooseMesh.h"

// libMesh
#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/quadrature.h"
#include "libmesh/dense_vector.h"
#include "libmesh/petsc_vector.h"

ArrayMooseVariable::ArrayMooseVariable(const std::string & name, unsigned int var_num, const FEType & fe_type, SystemBase & sys, Assembly & assembly, Moose::VarKindType var_kind, unsigned int count) :
    MooseVariableBase(name, var_num, fe_type, sys, assembly, var_kind, count),

    _qrule(_assembly.qRule()),
    _qrule_face(_assembly.qRuleFace()),
    _qrule_neighbor(_assembly.qRuleNeighbor()),
    _current_side(_assembly.side()),
    _neighbor(_assembly.neighbor()),

    _mapped_values(NULL, 0),
    _mapped_grad_phi(0),

    _need_u_old(false),
    _need_u_older(false),

    _need_grad_old(false),
    _need_grad_older(false),

//    _need_second(false),
//    _need_second_old(false),
//    _need_second_older(false),


    _need_u_old_neighbor(false),
    _need_u_older_neighbor(false),

    _need_grad_old_neighbor(false),
    _need_grad_older_neighbor(false),

//    _need_second_neighbor(false),
//    _need_second_old_neighbor(false),
//    _need_second_older_neighbor(false),

    _need_nodal_u(false),
    _need_nodal_u_old(false),
    _need_nodal_u_older(false),
    _need_nodal_u_dot(false),

    _need_nodal_u_neighbor(false),
    _need_nodal_u_old_neighbor(false),
    _need_nodal_u_older_neighbor(false),
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

ArrayMooseVariable::~ArrayMooseVariable()
{
  _u.release(); _u_bak.release();
  _u_old.release(); _u_old_bak.release();
  _u_older.release(); _u_older_bak.release();

  _grad_u.release(); _grad_u_bak.release();
  _grad_u_old.release(); _grad_u_old_bak.release();
  _grad_u_older.release(); _grad_u_older.release();

//  _second_u.release(); _second_u_bak.release();
//  _second_u_old.release(); _second_u_old_bak.release();
//  _second_u_older.release(); _second_u_older_bak.release();

  _u_dot.release(); _u_dot_bak.release();
  _u_dot_neighbor.release(); _u_dot_bak_neighbor.release();

  _du_dot_du.release(); _du_dot_du_bak.release();
  _du_dot_du_neighbor.release(); _du_dot_du_bak_neighbor.release();

  _nodal_u.release();
  _nodal_u_old.release();
  _nodal_u_older.release();
  _nodal_u_dot.release();
  _nodal_du_dot_du.release();

  _nodal_u_neighbor.release();
  _nodal_u_old_neighbor.release();
  _nodal_u_older_neighbor.release();
  _nodal_u_dot_neighbor.release();
  _nodal_du_dot_du_neighbor.release();

  _increment.release();

  _u_neighbor.release();
  _u_old_neighbor.release();
  _u_older_neighbor.release();

  _grad_u_neighbor.release();
  _grad_u_old_neighbor.release();
  _grad_u_older_neighbor.release();

//  _second_u_neighbor.release();
//  _second_u_old_neighbor.release();
//  _second_u_older_neighbor.release();
}


const std::set<SubdomainID> &
ArrayMooseVariable::activeSubdomains()
{
  return _sys.system().variable(_var_num).active_subdomains();
}

bool
ArrayMooseVariable::activeOnSubdomain(SubdomainID subdomain) const
{
  return _sys.system().variable(_var_num).active_on_subdomain(subdomain);
}

bool
ArrayMooseVariable::isNodal() const
{
  return _is_nodal;
}

void
ArrayMooseVariable::clearDofIndices()
{
  _dof_indices.clear();
}

void
ArrayMooseVariable::prepare()
{
  _dof_map.dof_indices (_elem, _dof_indices, _var_num);
  _has_nodal_value = false;
  _has_nodal_value_neighbor = false;

  // FIXME: remove this when the Richard's module is migrated to use the new NodalCoupleable interface.
  if (_dof_indices.size() > 0)
    _is_defined = true;
  else
    _is_defined = false;
}

void
ArrayMooseVariable::prepareNeighbor()
{
  _dof_map.dof_indices (_neighbor, _dof_indices_neighbor, _var_num);
  _has_nodal_value = false;
  _has_nodal_value_neighbor = false;
}

void
ArrayMooseVariable::prepareAux()
{
  _has_nodal_value = false;
  _has_nodal_value_neighbor = false;
}

void
ArrayMooseVariable::prepareIC()
{
  _dof_map.dof_indices(_elem, _dof_indices, _var_num);

  unsigned int nqp = _qrule->n_points();
  _u.resize(nqp);
}

void
ArrayMooseVariable::reinitNode()
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
ArrayMooseVariable::reinitNodeNeighbor()
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
ArrayMooseVariable::reinitAux()
{
  /* FIXME: this method is only for elemental auxiliary variables, so
   * we may want to rename it */
  _dof_map.dof_indices (_elem, _dof_indices, _var_num);
  if (_elem->n_dofs(_sys.number(), _var_num) > 0)
  {
    // FIXME: check if the following is equivalent with '_nodal_dof_index = _dof_indices[0];'?
    _nodal_dof_index = _elem->dof_number(_sys.number(), _var_num, 0);
    libmesh_assert(_dof_indices.size());
    _nodal_u.resize(_dof_indices.size());
//    _sys.currentSolution()->get(_dof_indices, &_nodal_u[0]);

    _is_defined = true;
  }
  else
    _is_defined = false;
}

void
ArrayMooseVariable::reinitAuxNeighbor()
{
  if (_neighbor)
  {
    _dof_map.dof_indices (_neighbor, _dof_indices_neighbor, _var_num);
    if (_neighbor->n_dofs(_sys.number(), _var_num) > 0)
    {
      _nodal_dof_index_neighbor = _neighbor->dof_number(_sys.number(), _var_num, 0);

      libmesh_assert(_dof_indices_neighbor.size());
      _nodal_u_neighbor.resize(_dof_indices_neighbor.size());
//      _sys.currentSolution()->get(_dof_indices_neighbor,
//                                  &_nodal_u_neighbor[0]);

      _is_defined_neighbor = true;
    }
    else
      _is_defined_neighbor = false;
  }
  else
    _is_defined_neighbor = false;
}

void
ArrayMooseVariable::reinitNodes(const std::vector<dof_id_type> & nodes)
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
ArrayMooseVariable::reinitNodesNeighbor(const std::vector<dof_id_type> & nodes)
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
ArrayMooseVariable::getDofIndices(const Elem * elem, std::vector<dof_id_type> & dof_indices)
{
  _dof_map.dof_indices(elem, dof_indices, _var_num);
}

// void
// ArrayMooseVariable::insert(NumericVector<Number> & residual)
// {
//   if (_has_nodal_value)
//     residual.insert(&_nodal_u[0], _dof_indices);
//
//   if (_has_nodal_value_neighbor)
//     residual.insert(&_nodal_u_neighbor[0], _dof_indices_neighbor);
// }

// void
// ArrayMooseVariable::add(NumericVector<Number> & residual)
// {
//   if (_has_nodal_value)
//     residual.add_vector(&_nodal_u[0], _dof_indices);
//
//   if (_has_nodal_value_neighbor)
//     residual.add_vector(&_nodal_u_neighbor[0], _dof_indices_neighbor);
// }

const VariablePhiValue &
ArrayMooseVariable::phi()
{
  return _phi;
}

const ArrayVariablePhiGradient &
ArrayMooseVariable::gradPhi()
{
  return _mapped_grad_phi;
}

// const VariablePhiSecond &
// ArrayMooseVariable::secondPhi()
// {
//   _second_phi = &_assembly.feSecondPhi(_fe_type);
//   return *_second_phi;
// }

const VariablePhiValue &
ArrayMooseVariable::phiFace()
{
  return _phi_face;
}

const VariablePhiGradient &
ArrayMooseVariable::gradPhiFace()
{
  return _grad_phi_face;
}

// const VariablePhiSecond &
// ArrayMooseVariable::secondPhiFace()
// {
//   _second_phi_face = &_assembly.feSecondPhiFace(_fe_type);
//   return *_second_phi_face;
// }

const VariablePhiValue &
ArrayMooseVariable::phiNeighbor()
{
  return _phi_neighbor;
}

const VariablePhiGradient &
ArrayMooseVariable::gradPhiNeighbor()
{
  return _grad_phi_neighbor;
}

// const VariablePhiSecond &
// ArrayMooseVariable::secondPhiNeighbor()
// {
//   _second_phi_neighbor = &_assembly.feSecondPhiNeighbor(_fe_type);
//   return *_second_phi_neighbor;
// }

const VariablePhiValue &
ArrayMooseVariable::phiFaceNeighbor()
{
  return _phi_face_neighbor;
}

const VariablePhiGradient &
ArrayMooseVariable::gradPhiFaceNeighbor()
{
  return _grad_phi_face_neighbor;
}

// const VariablePhiSecond &
// ArrayMooseVariable::secondPhiFaceNeighbor()
// {
//   _second_phi_face_neighbor = &_assembly.feSecondPhiFaceNeighbor(_fe_type);
//   return *_second_phi_face_neighbor;
// }

// const VariableValue &
// ArrayMooseVariable::nodalValue()
// {
//   if (isNodal())
//   {
//     _need_nodal_u = true;
//     return _nodal_u;
//   }
//   else
//     mooseError("Nodal values can be requested only on nodal variables, variable '" << name() << "' is not nodal.");
// }
//
// const VariableValue &
// ArrayMooseVariable::nodalValueOld()
// {
//   if (isNodal())
//   {
//     _need_nodal_u_old = true;
//     return _nodal_u_old;
//   }
//   else
//     mooseError("Nodal values can be requested only on nodal variables, variable '" << name() << "' is not nodal.");
// }
//
// const VariableValue &
// ArrayMooseVariable::nodalValueOlder()
// {
//   if (isNodal())
//   {
//     _need_nodal_u_older = true;
//     return _nodal_u_older;
//   }
//   else
//     mooseError("Nodal values can be requested only on nodal variables, variable '" << name() << "' is not nodal.");
// }
//
// const VariableValue &
// ArrayMooseVariable::nodalValueDot()
// {
//   if (isNodal())
//   {
//     _need_nodal_u_dot = true;
//     return _nodal_u_dot;
//   }
//   else
//     mooseError("Nodal values can be requested only on nodal variables, variable '" << name() << "' is not nodal.");
// }
//
// const VariableValue &
// ArrayMooseVariable::nodalValueNeighbor()
// {
//   if (isNodal())
//   {
//     _need_nodal_u_neighbor = true;
//     return _nodal_u_neighbor;
//   }
//   else
//     mooseError("Nodal values can be requested only on nodal variables, variable '" << name() << "' is not nodal.");
// }
//
// const VariableValue &
// ArrayMooseVariable::nodalValueOldNeighbor()
// {
//   if (isNodal())
//   {
//     _need_nodal_u_old_neighbor = true;
//     return _nodal_u_old_neighbor;
//   }
//   else
//     mooseError("Nodal values can be requested only on nodal variables, variable '" << name() << "' is not nodal.");
// }
//
// const VariableValue &
// ArrayMooseVariable::nodalValueOlderNeighbor()
// {
//   if (isNodal())
//   {
//     _need_nodal_u_older_neighbor = true;
//     return _nodal_u_older_neighbor;
//   }
//   else
//     mooseError("Nodal values can be requested only on nodal variables, variable '" << name() << "' is not nodal.");
// }
//
// const VariableValue &
// ArrayMooseVariable::nodalValueDotNeighbor()
// {
//   if (isNodal())
//   {
//     _need_nodal_u_dot_neighbor = true;
//     return _nodal_u_dot_neighbor;
//   }
//   else
//     mooseError("Nodal values can be requested only on nodal variables, variable '" << name() << "' is not nodal.");
// }

void
ArrayMooseVariable::computeElemValues()
{

  bool is_transient = _subproblem.isTransient();
  unsigned int n_qp = _qrule->n_points();
  unsigned int n_shapes = _dof_indices.size();

  auto var_group = _sys.system().variable_group(0);
  unsigned int n_vars = var_group.n_variables();

  const PetscVector<Real> & current_solution = dynamic_cast<const PetscVector<Real>&>(*_sys.currentSolution());
  const PetscVector<Real> & solution_old     = dynamic_cast<const PetscVector<Real>&>(_sys.solutionOld());
  const PetscVector<Real> & solution_older   = dynamic_cast<const PetscVector<Real>&>(_sys.solutionOlder());
  const PetscVector<Real> & u_dot            = dynamic_cast<const PetscVector<Real>&>(_sys.solutionUDot());
  const Real & du_dot_du                       = _sys.duDotDu();

  // We need to get array only for reading... but we need to be able to pass the raw pointer into an Eigen::Map
  // Eigen::Map requires a non-const pointer... so we're going to const_cast it here
  // (but we're still NOT going to modify it).
  PetscScalar * current_solution_values = const_cast<PetscScalar *>(current_solution.get_array_read());

  _u.resize(n_qp);
  for (unsigned int qp = 0; qp < n_qp; qp++)
  {
    auto & qp_value = _u[qp];

    qp_value.resize(n_vars);
    qp_value.setZero();
  }

  _grad_u.resize(n_qp);
  for (unsigned int qp = 0; qp < n_qp; qp++)
  {
    auto & qp_value = _grad_u[qp];

    qp_value.resize(n_vars, Eigen::NoChange);
    qp_value.setZero();
  }

  auto n_dofs = _dof_indices.size();

  // Global to local map the dof indices
  _local_dof_indices.resize(n_dofs);
  for (unsigned int i = 0; i < n_dofs; i++)
    _local_dof_indices[i] = current_solution.map_global_to_local_index(_dof_indices[i]);

  // Map grad_phi using Eigen so that we can reference it in the normal way
  _mapped_grad_phi.resize(n_dofs);
  for (unsigned int i = 0; i < n_dofs; i++)
  {
    _mapped_grad_phi[i].resize(n_qp, NULL);
    for (unsigned int qp = 0; qp < n_qp; qp++)
      // Note to future self: this does NOT do any allocation.  It is "reconstructing" the object in place and it is FAST
      new (&_mapped_grad_phi[i][qp]) Eigen::Map<Eigen::Vector3d>(const_cast<Real *>(&_grad_phi[i][qp](0)));
  }

  for (unsigned int i = 0; i < n_shapes; i++)
  {
    auto coefficients_start = current_solution_values + _local_dof_indices[i];

    // Same here (no allocation actually done)
    new (&_mapped_values) Eigen::Map<Eigen::VectorXd>(coefficients_start, n_vars);

    // Note: "noalias" means that the LHS is not modified by the RHS... so more optimization can be done!
    for (unsigned int qp = 0; qp < n_qp; qp++)
      _u[qp].noalias() += _mapped_values * _phi[i][qp];

    // This looks like a good idea... but it turns out it's slow as dirt:
    //
    // for (unsigned int qp = 0; qp < n_qp; qp++)
    //  _grad_u[qp].noalias() += _mapped_values * _mapped_grad_phi[i][qp].transpose();
    //
    // This one is quite a bit better:
    //
    //  _grad_u[qp].col(0).noalias() += _mapped_values * grad_phi[i][qp](0);
    //  _grad_u[qp].col(1).noalias() += _mapped_values * grad_phi[i][qp](1);
    //  _grad_u[qp].col(2).noalias() += _mapped_values * grad_phi[i][qp](2);
    //
    // But still not quite optimal.
    //
    // SO: I've rolled my own instead (meticulously optimized in an external code)
    // All this does is compute the gradient of each variable by summing into the columns
    //
    // Note: No reason to roll my own for the computation of _u above... it's perfectly fast
    //
    // Note: Eigen Matrix objects are _column major_ by default... which helps us here!
    for (unsigned int qp = 0; qp < n_qp; qp++)
    {
      // Grab the raw (column major) grad_u data
      auto grad_u_data = _grad_u[qp].data();

      for (unsigned int col = 0; col < LIBMESH_DIM; col++)
      {
        auto current_col_start = grad_u_data + (col * n_vars);

        auto grad_val = _grad_phi[i][qp](col);

        // As the number of variables goes up... this gets awesome
        // It's zipping down an entire column and doing FMAdd (Fused Multiply Add) operations
        // On Haswell+ this can rock and roll...
        for (auto row = decltype(n_vars)(0); row < n_vars; row++)
          current_col_start[row] += coefficients_start[row] * grad_val;
      }
    }
  }

  const_cast<PetscVector<Real> &>(current_solution).restore_array();
}

// void
// ArrayMooseVariable::computeElemValuesFace()
// {
//   bool is_transient = _subproblem.isTransient();
//   unsigned int nqp = _qrule_face->n_points();
//   _u.resize(nqp);
//   _grad_u.resize(nqp);

//   if (_need_second)
//     _second_u.resize(nqp);

//   if (is_transient)
//   {
//     _u_dot.resize(nqp);
//     _du_dot_du.resize(nqp);

//     if (_need_u_old)
//       _u_old.resize(nqp);

//     if (_need_u_older)
//       _u_older.resize(nqp);

//     if (_need_grad_old)
//       _grad_u_old.resize(nqp);

//     if (_need_grad_older)
//       _grad_u_older.resize(nqp);

//     if (_need_second_old)
//       _second_u_old.resize(nqp);

//     if (_need_second_older)
//       _second_u_older.resize(nqp);
//   }

//   for (unsigned int i = 0; i < nqp; ++i)
//   {
//     _u[i] = 0;
//     _grad_u[i] = 0;

//     if (_need_second)
//       _second_u[i] = 0;

//     if (_subproblem.isTransient())
//     {
//       _u_dot[i] = 0;
//       _du_dot_du[i] = 0;

//       if (_need_u_old)
//         _u_old[i] = 0;

//       if (_need_u_older)
//         _u_older[i] = 0;

//       if (_need_grad_old)
//         _grad_u_old[i] = 0;

//       if (_need_grad_older)
//         _grad_u_older[i] = 0;

//       if (_need_second_old)
//         _second_u_old[i] = 0;

//       if (_need_second_older)
//         _second_u_older[i] = 0;
//     }
//   }

//   unsigned int num_dofs = _dof_indices.size();

//   if (_need_nodal_u)
//     _nodal_u.resize(num_dofs);
//   if (is_transient)
//   {
//     if (_need_nodal_u_old)
//       _nodal_u_old.resize(num_dofs);
//     if (_need_nodal_u_older)
//       _nodal_u_older.resize(num_dofs);
//     if (_need_nodal_u_dot)
//       _nodal_u_dot.resize(num_dofs);
//   }

//   const NumericVector<Real> & crruent_solution = *_sys.currentSolution();
//   const NumericVector<Real> & solution_old     = _sys.solutionOld();
//   const NumericVector<Real> & solution_older   = _sys.solutionOlder();
//   const NumericVector<Real> & u_dot            = _sys.solutionUDot();
//   const Real & du_dot_du                       = _sys.duDotDu();

//   dof_id_type idx;
//   Real soln_local;
//   Real soln_old_local = 0;
//   Real soln_older_local = 0;
//   Real u_dot_local = 0;

//   Real phi_local;
//   RealGradient dphi_local;
//   RealTensor d2phi_local;

//   for (unsigned int i=0; i < num_dofs; ++i)
//   {
//     idx = _dof_indices[i];
//     soln_local = current_solution(idx);

//     if (_need_nodal_u)
//       _nodal_u[i] = soln_local;

//     if (is_transient)
//     {
//       if (_need_u_old || _need_grad_old || _need_second_old || _need_nodal_u_old)
//         soln_old_local = solution_old(idx);

//       if (_need_u_older || _need_grad_older || _need_second_older || _need_nodal_u_older)
//         soln_older_local = solution_older(idx);

//       if (_need_nodal_u_old)
//         _nodal_u_old[i] = soln_old_local;
//       if (_need_nodal_u_older)
//         _nodal_u_older[i] = soln_older_local;

//       u_dot_local = u_dot(idx);
//       if (_need_nodal_u_dot)
//         _nodal_u_dot[i] = u_dot_local;
//     }

//     for (unsigned int qp=0; qp < nqp; ++qp)
//     {
//       phi_local = _phi_face[i][qp];
//       dphi_local = _grad_phi_face[i][qp];

//       if (_need_second || _need_second_old || _need_second_older)
//         d2phi_local = (*_second_phi_face)[i][qp];

//       _u[qp]      += phi_local * soln_local;
//       _grad_u[qp] += dphi_local * soln_local;

//       if (_need_second)
//         _second_u[qp] += d2phi_local * soln_local;

//       if (is_transient)
//       {
//         _u_dot[qp]        += phi_local * u_dot_local;
//         _du_dot_du[qp]    = du_dot_du;

//         if (_need_u_old)
//           _u_old[qp]        += phi_local * soln_old_local;

//         if (_need_u_older)
//           _u_older[qp]      += phi_local * soln_older_local;

//         if (_need_grad_old)
//           _grad_u_old[qp]   += dphi_local * soln_old_local;

//         if (_need_grad_older)
//           _grad_u_older[qp] += dphi_local * soln_older_local;

//         if (_need_second_old)
//           _second_u_old[qp] += d2phi_local * soln_old_local;

//         if (_need_second_older)
//           _second_u_older[qp] += d2phi_local * soln_older_local;
//       }
//     }
//   }
// }

// void
// ArrayMooseVariable::computeNeighborValuesFace()
// {
//   bool is_transient = _subproblem.isTransient();
//   unsigned int nqp = _qrule_neighbor->n_points();

//   _u_neighbor.resize(nqp);
//   _grad_u_neighbor.resize(nqp);

//   if (_need_second_neighbor)
//     _second_u_neighbor.resize(nqp);

//   if (is_transient)
//   {
//     _u_dot_neighbor.resize(nqp);
//     _du_dot_du_neighbor.resize(nqp);

//     if (_need_u_old_neighbor)
//       _u_old_neighbor.resize(nqp);

//     if (_need_u_older_neighbor)
//       _u_older_neighbor.resize(nqp);

//     if (_need_grad_old_neighbor)
//       _grad_u_old_neighbor.resize(nqp);

//     if (_need_grad_older_neighbor)
//       _grad_u_older_neighbor.resize(nqp);

//     if (_need_second_old_neighbor)
//       _second_u_old_neighbor.resize(nqp);

//     if (_need_second_older_neighbor)
//       _second_u_older_neighbor.resize(nqp);
//   }

//   for (unsigned int i = 0; i < nqp; ++i)
//   {
//     _u_neighbor[i] = 0;
//     _grad_u_neighbor[i] = 0;

//     if (_need_second_neighbor)
//       _second_u_neighbor[i] = 0;

//     if (_subproblem.isTransient())
//     {
//       _u_dot_neighbor[i] = 0;
//       _du_dot_du_neighbor[i] = 0;

//       if (_need_u_old_neighbor)
//         _u_old_neighbor[i] = 0;

//       if (_need_u_older_neighbor)
//         _u_older_neighbor[i] = 0;

//       if (_need_grad_old_neighbor)
//         _grad_u_old_neighbor[i] = 0;

//       if (_need_grad_older_neighbor)
//         _grad_u_older_neighbor[i] = 0;

//       if (_need_second_old_neighbor)
//         _second_u_old_neighbor[i] = 0;

//       if (_need_second_older_neighbor)
//         _second_u_older_neighbor[i] = 0;
//     }
//   }

//   unsigned int num_dofs = _dof_indices_neighbor.size();

//   if (_need_nodal_u_neighbor)
//     _nodal_u_neighbor.resize(num_dofs);
//   if (is_transient)
//   {
//     if (_need_nodal_u_old_neighbor)
//       _nodal_u_old_neighbor.resize(num_dofs);
//     if (_need_nodal_u_older_neighbor)
//       _nodal_u_older_neighbor.resize(num_dofs);
//     if (_need_nodal_u_dot_neighbor)
//       _nodal_u_dot_neighbor.resize(num_dofs);
//   }

//   const NumericVector<Real> & current_solution = *_sys.currentSolution();
//   const NumericVector<Real> & solution_old     = _sys.solutionOld();
//   const NumericVector<Real> & solution_older   = _sys.solutionOlder();
//   const NumericVector<Real> & u_dot            = _sys.solutionUDot();
//   const Real & du_dot_du                       = _sys.duDotDu();

//   dof_id_type idx;
//   Real soln_local;
//   Real soln_old_local=0;
//   Real soln_older_local=0;
//   Real u_dot_local = 0;

//   Real phi_local;
//   RealGradient dphi_local;
//   RealTensor d2phi_local;

//   for (unsigned int i=0; i < num_dofs; ++i)
//   {
//     idx = _dof_indices_neighbor[i];
//     soln_local = current_solution(idx);

//     if (is_transient)
//     {
//       if (_need_u_old_neighbor || _need_grad_old_neighbor || _need_second_old_neighbor)
//         soln_old_local = solution_old(idx);

//       if (_need_u_older_neighbor || _need_grad_older_neighbor || _need_second_older_neighbor)
//         soln_older_local = solution_older(idx);

//       if (_need_nodal_u_old_neighbor)
//         _nodal_u_old_neighbor[i] = soln_old_local;
//       if (_need_nodal_u_older_neighbor)
//         _nodal_u_older_neighbor[i] = soln_older_local;

//       u_dot_local = u_dot(idx);
//       if (_need_nodal_u_dot_neighbor)
//         _nodal_u_dot_neighbor[i] = u_dot_local;
//     }

//     for (unsigned int qp=0; qp < nqp; ++qp)
//     {
//       phi_local = _phi_face_neighbor[i][qp];
//       dphi_local = _grad_phi_face_neighbor[i][qp];

//       if (_need_second_neighbor || _need_second_old_neighbor || _need_second_older_neighbor)
//         d2phi_local = (*_second_phi_face_neighbor)[i][qp];

//       _u_neighbor[qp]      += phi_local * soln_local;
//       _grad_u_neighbor[qp] += dphi_local * soln_local;

//       if (_need_second_neighbor)
//         _second_u_neighbor[qp] += d2phi_local * soln_local;

//       if (is_transient)
//       {
//         _u_dot_neighbor[qp]        += phi_local * u_dot_local;
//         _du_dot_du_neighbor[qp]    = du_dot_du;

//         if (_need_u_old_neighbor)
//           _u_old_neighbor[qp]        += phi_local * soln_old_local;

//         if (_need_u_older_neighbor)
//           _u_older_neighbor[qp]      += phi_local * soln_older_local;

//         if (_need_grad_old_neighbor)
//           _grad_u_old_neighbor[qp]   += dphi_local * soln_old_local;

//         if (_need_grad_older_neighbor)
//           _grad_u_older_neighbor[qp]   += dphi_local * soln_older_local;

//         if (_need_second_old_neighbor)
//           _second_u_old_neighbor[qp] += d2phi_local * soln_old_local;

//         if (_need_second_older_neighbor)
//           _second_u_older_neighbor[qp] += d2phi_local * soln_older_local;
//       }
//     }
//   }
// }

// void
// ArrayMooseVariable::computeNeighborValues()
// {
//   bool is_transient = _subproblem.isTransient();
//   unsigned int nqp = _qrule_neighbor->n_points();

//   _u_neighbor.resize(nqp);
//   _grad_u_neighbor.resize(nqp);

//   if (_need_second_neighbor)
//     _second_u_neighbor.resize(nqp);

//   if (is_transient)
//   {
//     if (_need_u_old_neighbor)
//       _u_old_neighbor.resize(nqp);

//     if (_need_u_older_neighbor)
//       _u_older_neighbor.resize(nqp);

//     if (_need_grad_old_neighbor)
//       _grad_u_old_neighbor.resize(nqp);

//     if (_need_grad_older_neighbor)
//       _grad_u_older_neighbor.resize(nqp);

//     if (_need_second_old_neighbor)
//       _second_u_old_neighbor.resize(nqp);

//     if (_need_second_older_neighbor)
//       _second_u_older_neighbor.resize(nqp);
//   }

//   for (unsigned int i = 0; i < nqp; ++i)
//   {
//     _u_neighbor[i] = 0;
//     _grad_u_neighbor[i] = 0;

//     if (_need_second_neighbor)
//       _second_u_neighbor[i] = 0;

//     if (_subproblem.isTransient())
//     {
//       if (_need_u_old_neighbor)
//         _u_old_neighbor[i] = 0;

//       if (_need_u_older_neighbor)
//         _u_older_neighbor[i] = 0;

//       if (_need_grad_old_neighbor)
//         _grad_u_old_neighbor[i] = 0;

//       if (_need_grad_older_neighbor)
//         _grad_u_older_neighbor[i] = 0;

//       if (_need_second_old_neighbor)
//         _second_u_old_neighbor[i] = 0;

//       if (_need_second_older_neighbor)
//         _second_u_older_neighbor[i] = 0;
//     }
//   }

//   unsigned int num_dofs = _dof_indices_neighbor.size();

//   if (_need_nodal_u_neighbor)
//     _nodal_u_neighbor.resize(num_dofs);
//   if (is_transient)
//   {
//     if (_need_nodal_u_old_neighbor)
//       _nodal_u_old_neighbor.resize(num_dofs);
//     if (_need_nodal_u_older_neighbor)
//       _nodal_u_older_neighbor.resize(num_dofs);
//     if (_need_nodal_u_dot_neighbor)
//       _nodal_u_dot_neighbor.resize(num_dofs);
//   }

//   const NumericVector<Real> & current_solution = *_sys.currentSolution();
//   const NumericVector<Real> & solution_old     = _sys.solutionOld();
//   const NumericVector<Real> & solution_older   = _sys.solutionOlder();
//   const NumericVector<Real> & u_dot            = _sys.solutionUDot();

//   dof_id_type idx;
//   Real soln_local;
//   Real soln_old_local=0;
//   Real soln_older_local=0;
//   Real u_dot_local = 0;

//   Real phi_local;
//   RealGradient dphi_local;
//   RealTensor d2phi_local;

//   for (unsigned int i=0; i < num_dofs; ++i)
//   {
//     idx = _dof_indices_neighbor[i];
//     soln_local = current_solution(idx);

//     if (is_transient)
//     {
//       if (_need_u_old_neighbor)
//         soln_old_local = solution_old(idx);

//       if (_need_u_older_neighbor)
//         soln_older_local = solution_older(idx);

//       if (_need_nodal_u_old_neighbor)
//         _nodal_u_old_neighbor[i] = soln_old_local;
//       if (_need_nodal_u_older_neighbor)
//         _nodal_u_older_neighbor[i] = soln_older_local;

//       u_dot_local = u_dot(idx);
//       if (_need_nodal_u_dot_neighbor)
//         _nodal_u_dot_neighbor[i] = u_dot_local;
//     }

//     for (unsigned int qp=0; qp < nqp; ++qp)
//     {
//       phi_local = _phi_neighbor[i][qp];
//       dphi_local = _grad_phi_neighbor[i][qp];

//       if (_need_second_neighbor || _need_second_old_neighbor || _need_second_older_neighbor)
//         d2phi_local = (*_second_phi_neighbor)[i][qp];

//       _u_neighbor[qp]      += phi_local * soln_local;
//       _grad_u_neighbor[qp] += dphi_local * soln_local;

//       if (_need_second_neighbor)
//         _second_u_neighbor[qp] += d2phi_local * soln_local;

//       if (is_transient)
//       {
//         if (_need_u_old_neighbor)
//           _u_old_neighbor[qp]        += phi_local * soln_old_local;

//         if (_need_u_older_neighbor)
//           _u_older_neighbor[qp]      += phi_local * soln_older_local;

//         if (_need_grad_old_neighbor)
//           _grad_u_old_neighbor[qp]   += dphi_local * soln_old_local;

//         if (_need_grad_older_neighbor)
//           _grad_u_older_neighbor[qp]   += dphi_local * soln_older_local;

//         if (_need_second_old_neighbor)
//           _second_u_old_neighbor[qp] += d2phi_local * soln_old_local;

//         if (_need_second_older_neighbor)
//           _second_u_older_neighbor[qp] += d2phi_local * soln_older_local;
//       }
//     }
//   }
// }

void
ArrayMooseVariable::computeNodalValues()
{
  if (_is_defined)
  {
    _nodal_u.resize(1);
    _nodal_u[0].resize(_count);

    auto & petsc_solution = dynamic_cast<const PetscVector<Number> &>(*_sys.currentSolution());

    auto local_nodal_dof_index = petsc_solution.map_global_to_local_index(_nodal_dof_index);

    // About the const_cast: See the explanation for the same thing in computeElemValues()
    auto solution_values = const_cast<PetscScalar *>(petsc_solution.get_array_read());

    // Ok - nodalSln() must return the same type as sln() so, for now, I'm going to do this copy here
    // It would be much more awesome to just Eigen::Map the values directly... but to do that I would
    // have to change the type of _u to be a Map and then I would have to compute the values into an
    // Eigen::Vector (which I would store elsewhere) and then Map them.  That sounds like no fun...
    // so for now I'm just doing a copy...
    std::memcpy(_nodal_u[0].data(), solution_values + local_nodal_dof_index, _count * sizeof(Real));

    const_cast<PetscVector<Real> &>(petsc_solution).restore_array();
  }
  else
  {
    _nodal_u.resize(0);
    if (_subproblem.isTransient())
    {
      _nodal_u_old.resize(0);
      _nodal_u_older.resize(0);
      _nodal_u_dot.resize(0);
      _nodal_du_dot_du.resize(0);
    }
  }
}

// void
// ArrayMooseVariable::computeNodalNeighborValues()
// {
//   if (_is_defined_neighbor)
//   {
//     const unsigned int n = _dof_indices_neighbor.size();
//     mooseAssert(n, "Defined but empty?");
//     _nodal_u_neighbor.resize(n);
//     _sys.currentSolution()->get(_dof_indices_neighbor, &_nodal_u_neighbor[0]);

//     if (_subproblem.isTransient())
//     {
//       _nodal_u_old_neighbor.resize(n);
//       _nodal_u_older_neighbor.resize(n);
//       _sys.solutionOld().get(_dof_indices_neighbor, &_nodal_u_old_neighbor[0]);
//       _sys.solutionOlder().get(_dof_indices_neighbor, &_nodal_u_older_neighbor[0]);

//       _nodal_u_dot_neighbor.resize(n);
//       _nodal_du_dot_du_neighbor.resize(n);
//       for (unsigned int i = 0; i < n; i++)
//       {
//         _nodal_u_dot_neighbor[i] = _sys.solutionUDot()(_dof_indices_neighbor[i]);
//         _nodal_du_dot_du_neighbor[i] = _sys.duDotDu();
//       }
//     }
//   }
//   else
//   {
//     _nodal_u_neighbor.resize(0);
//     if (_subproblem.isTransient())
//     {
//       _nodal_u_old_neighbor.resize(0);
//       _nodal_u_older_neighbor.resize(0);
//       _nodal_u_dot_neighbor.resize(0);
//       _nodal_du_dot_du_neighbor.resize(0);
//     }
//   }
// }

// void
// ArrayMooseVariable::setNodalValue(const DenseVector<Number> & values)
// {
//   for (unsigned int i = 0; i < values.size(); i++)
//     _nodal_u[i] = values(i);

//   _has_nodal_value = true;

//   for (unsigned int qp = 0; qp < _u.size(); qp++)
//   {
//     _u[qp] = 0;
//     for (unsigned int i = 0; i < _nodal_u.size(); i++)
//       _u[qp] += _phi[i][qp] * _nodal_u[i];
//   }
// }

// void
// ArrayMooseVariable::setNodalValueNeighbor(const DenseVector<Number> & values)
// {
//   for (unsigned int i=0; i<values.size(); i++)
//     _nodal_u_neighbor[i] = values(i);
//   _has_nodal_value_neighbor = true;

//   if (isNodal())
//     mooseError("Variable " + name() + " has to be nodal!");
//   else
//   {
//     for (unsigned int qp=0; qp<_u_neighbor.size(); qp++)
//     {
//       _u_neighbor[qp] = 0;
//       for (unsigned int i=0; i < _nodal_u_neighbor.size(); i++)
//         _u_neighbor[qp] = _phi_face_neighbor[i][qp] * _nodal_u_neighbor[i];
//     }
//   }
// }

// void
// ArrayMooseVariable::setNodalValue(Number value, unsigned int idx/* = 0*/)
// {
//   _nodal_u[idx] = value;                  // update variable nodal value
//   _has_nodal_value = true;

//   // Update the qp values as well
//   for (unsigned int qp = 0; qp < _u.size(); qp++)
//     _u[qp] = value;
// }

// void
// ArrayMooseVariable::setNodalValueNeighbor(Number value)
// {
//   _nodal_u_neighbor[0] = value;                  // update variable nodal value
//   _has_nodal_value_neighbor = true;

//   if (!isNodal()) // If this is an elemental variable, then update the qp values as well
//   {
//     for (unsigned int qp=0; qp<_u_neighbor.size(); qp++)
//       _u_neighbor[qp] = value;
//   }
// }


// void
// ArrayMooseVariable::computeIncrement(const NumericVector<Number> & increment_vec)
// {
//   unsigned int nqp = _qrule->n_points();
//
//   _increment.resize(nqp);
//   // Compute the increment at each quadrature point
//   unsigned int num_dofs = _dof_indices.size();
//   for (unsigned int qp=0; qp<nqp; qp++)
//   {
//     _increment[qp]=0;
//     for (unsigned int i=0; i<num_dofs; i++)
//       _increment[qp] +=  _phi[i][qp]*increment_vec(_dof_indices[i]);
//   }
// }

// Number
// ArrayMooseVariable::getNodalValue(const Node & node)
// {
//   mooseAssert(_subproblem.mesh().isSemiLocal(const_cast<Node *>(&node)), "Node is not Semilocal");
//
//   // Make sure that the node has DOFs
//   /* Note, this is a reproduction of an assert within libMesh::Node::dof_number, this is done to produce a
//    * better error (see misc/check_error.node_value_off_block) */
//   mooseAssert(node.n_dofs(_sys.number(), _var_num) > 0, "Node " << node.id() << " does not contain any dofs for the " << _sys.system().variable_name(_var_num) << " variable");
//
//   dof_id_type dof = node.dof_number(_sys.number(), _var_num, 0);
//
//   return (*_sys.currentSolution())(dof);
// }

// Number
// ArrayMooseVariable::getNodalValueOld(const Node & node)
// {
//   mooseAssert(_subproblem.mesh().isSemiLocal(const_cast<Node *>(&node)), "Node is not Semilocal");
//
//   // Make sure that the node has DOFs
//   /* Note, this is a reproduction of an assert within libMesh::Node::dof_number, this is done to produce a
//    * better error (see misc/check_error.node_value_off_block) */
//   mooseAssert(node.n_dofs(_sys.number(), _var_num) > 0, "Node " << node.id() << " does not contain any dofs for the " << _sys.system().variable_name(_var_num) << " variable");
//
//   dof_id_type dof = node.dof_number(_sys.number(), _var_num, 0);
//   return _sys.solutionOld()(dof);
// }
//
// Number
// ArrayMooseVariable::getNodalValueOlder(const Node & node)
// {
//   mooseAssert(_subproblem.mesh().isSemiLocal(const_cast<Node *>(&node)), "Node is not Semilocal");
//
//   // Make sure that the node has DOFs
//   /* Note, this is a reproduction of an assert within libMesh::Node::dof_number, this is done to produce a
//    * better error (see misc/check_error.node_value_off_block) */
//   mooseAssert(node.n_dofs(_sys.number(), _var_num) > 0, "Node " << node.id() << " does not contain any dofs for the " << _sys.system().variable_name(_var_num) << " variable");
//
//   dof_id_type dof = node.dof_number(_sys.number(), _var_num, 0);
//   return _sys.solutionOlder()(dof);
// }
//
// Real
// ArrayMooseVariable::getValue(const Elem * elem, const std::vector<std::vector<Real> > & phi) const
// {
//   std::vector<dof_id_type> dof_indices;
//   _dof_map.dof_indices(elem, dof_indices, _var_num);
//
//   Real value = 0;
//   if (isNodal())
//   {
//     for (unsigned int i = 0; i < dof_indices.size(); ++i)
//     {
//       //The zero index is because we only have one point that the phis are evaluated at
//       value += phi[i][0] * (*_sys.currentSolution())(dof_indices[i]);
//     }
//   }
//   else
//   {
//     mooseAssert(dof_indices.size() == 1, "Wrong size for dof indices");
//     value = (*_sys.currentSolution())(dof_indices[0]);
//   }
//
//   return value;
// }
//
// RealGradient
// ArrayMooseVariable::getGradient(const Elem * elem, const std::vector<std::vector<RealGradient> > & grad_phi) const
// {
//   std::vector<dof_id_type> dof_indices;
//   _dof_map.dof_indices(elem, dof_indices, _var_num);
//
//   RealGradient value;
//   if (isNodal())
//   {
//     for (unsigned int i = 0; i < dof_indices.size(); ++i)
//     {
//       //The zero index is because we only have one point that the phis are evaluated at
//       value += grad_phi[i][0] * (*_sys.currentSolution())(dof_indices[i]);
//     }
//   }
//   else
//   {
//     mooseAssert(dof_indices.size() == 1, "Wrong size for dof indices");
//     value = 0.0;
//   }
//
//   return value;
// }
//
// Real
// ArrayMooseVariable::getElementalValue(const Elem * elem, unsigned int idx) const
// {
//   std::vector<dof_id_type> dof_indices;
//   _dof_map.dof_indices(elem, dof_indices, _var_num);
//
//   return (*_sys.currentSolution())(dof_indices[idx]);
// }
