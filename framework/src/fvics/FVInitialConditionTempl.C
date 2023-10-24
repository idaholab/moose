//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInitialConditionTempl.h"
#include "FEProblem.h"
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/fe_interface.h"
#include "libmesh/quadrature.h"

template <typename T>
FVInitialConditionTempl<T>::FVInitialConditionTempl(const InputParameters & parameters)
  : InitialConditionBase(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _tid(getParam<THREAD_ID>("_tid")),
    _t(_fe_problem.time()),
    _base_var(_sys.getActualFieldVariable<T>(parameters.get<THREAD_ID>("_tid"),
                                             parameters.get<VariableName>("variable")))
{
  if (!_base_var.isFV())
    mooseError("A finite volume initial condition has been defined for variable '",
               _base_var.name(),
               "' whereas the variable itself is not a finite volume variable!");
}

template <typename T>
FVInitialConditionTempl<T>::~FVInitialConditionTempl()
{
}

template <typename T>
void
FVInitialConditionTempl<T>::compute()
{
  std::cout << "Something wil lcome here eventually" << std::endl;
  // // -- NOTE ----
  // // The following code is a copy from libMesh project_vector.C plus it adds some features, so we
  // // can couple variable values
  // // and we also do not call any callbacks, but we use our initial condition system directly.
  // // ------------

  // // The dimension of the current element
  // _dim = _current_elem->dim();
  // // The element type
  // const ElemType elem_type = _current_elem->type();
  // // The number of nodes on the new element
  // const unsigned int n_nodes = _current_elem->n_nodes();

  // // Get FE objects of the appropriate type
  // // We cannot use the FE object in Assembly, since the following code is messing with the
  // // quadrature rules
  // // for projections and would screw it up. However, if we implement projections from one mesh to
  // // another,
  // // this code should use that implementation.
  // std::unique_ptr<FEBaseType> fe(FEBaseType::build(_dim, _fe_type));

  // // Prepare variables for projection
  // std::unique_ptr<QBase> qrule(_fe_type.default_quadrature_rule(_dim));
  // std::unique_ptr<QBase> qedgerule(_fe_type.default_quadrature_rule(1));
  // std::unique_ptr<QBase> qsiderule(_fe_type.default_quadrature_rule(_dim - 1));

  // // The values of the shape functions at the quadrature points
  // _phi = &fe->get_phi();

  // // The gradients of the shape functions at the quadrature points on the child element.
  // _dphi = nullptr;

  // _cont = fe->get_continuity();

  // if (_cont == C_ONE)
  // {
  //   const std::vector<std::vector<GradientShapeType>> & ref_dphi = fe->get_dphi();
  //   _dphi = &ref_dphi;
  // }

  // // The Jacobian * quadrature weight at the quadrature points
  // _JxW = &fe->get_JxW();
  // // The XYZ locations of the quadrature points
  // _xyz_values = &fe->get_xyz();

  // // Update the DOF indices for this element based on the current mesh
  // _var.prepareIC();
  // _dof_indices = _var.dofIndices();

  // // The number of DOFs on the element
  // const unsigned int n_dofs = _dof_indices.size();
  // if (n_dofs == 0)
  //   return;

  // // Fixed vs. free DoFs on edge/face projections
  // _dof_is_fixed.clear();
  // _dof_is_fixed.resize(n_dofs, false);
  // _free_dof.clear();
  // _free_dof.resize(n_dofs, 0);

  // // Zero the interpolated values
  // _Ue.resize(n_dofs);
  // _Ue.zero();

  // DenseVector<char> mask(n_dofs, true);

  // // In general, we need a series of
  // // projections to ensure a unique and continuous
  // // solution.  We start by interpolating nodes, then
  // // hold those fixed and project edges, then
  // // hold those fixed and project faces, then
  // // hold those fixed and project interiors

  // // Interpolate node values first
  // _current_dof = 0;

  // for (_n = 0; _n != n_nodes; ++_n)
  // {
  //   _nc = FEInterface::n_dofs_at_node(_dim, _fe_type, elem_type, _n);

  //   // for nodes that are in more than one subdomain, only compute the initial
  //   // condition once on the lowest numbered block
  //   auto curr_node = _current_elem->node_ptr(_n);
  //   const auto & block_ids = _sys.mesh().getNodeBlockIds(*curr_node);

  //   auto priority_block = *(block_ids.begin());
  //   for (auto id : block_ids)
  //     if (_var.hasBlocks(id))
  //     {
  //       priority_block = id;
  //       break;
  //     }

  //   if (!hasBlocks(priority_block) && _var.isNodal())
  //   {
  //     for (decltype(_nc) i = 0; i < _nc; ++i)
  //     {
  //       mask(_current_dof) = false;
  //       _current_dof++;
  //     }
  //     continue;
  //   }

  //   // FIXME: this should go through the DofMap,
  //   // not duplicate _dof_indices code badly!
  //   if (!_current_elem->is_vertex(_n))
  //   {
  //     _current_dof += _nc;
  //     continue;
  //   }

  //   if (_cont == DISCONTINUOUS)
  //     libmesh_assert(_nc == 0);
  //   else if (_cont == C_ZERO)
  //     setCZeroVertices();
  //   else if (_fe_type.family == HERMITE)
  //     setHermiteVertices();
  //   else if (_cont == C_ONE)
  //     setOtherCOneVertices();
  //   else if (_cont == SIDE_DISCONTINUOUS)
  //     continue;
  //   else
  //     libmesh_error();
  // } // loop over nodes

  // // From here on out we won't be sampling at nodes anymore
  // _current_node = nullptr;

  // // In 3D, project any edge values next
  // if (_dim > 2 && _cont != DISCONTINUOUS)
  //   for (unsigned int e = 0; e != _current_elem->n_edges(); ++e)
  //   {
  //     FEInterface::dofs_on_edge(_current_elem, _dim, _fe_type, e, _side_dofs);

  //     // Some edge dofs are on nodes and already
  //     // fixed, others are free to calculate
  //     _free_dofs = 0;
  //     for (unsigned int i = 0; i != _side_dofs.size(); ++i)
  //       if (!_dof_is_fixed[_side_dofs[i]])
  //         _free_dof[_free_dofs++] = i;

  //     // There may be nothing to project
  //     if (!_free_dofs)
  //       continue;

  //     // Initialize FE data on the edge
  //     fe->attach_quadrature_rule(qedgerule.get());
  //     fe->edge_reinit(_current_elem, e);
  //     _n_qp = qedgerule->n_points();

  //     choleskySolve(false);
  //   }

  // // Project any side values (edges in 2D, faces in 3D)
  // if (_dim > 1 && _cont != DISCONTINUOUS)
  //   for (unsigned int s = 0; s != _current_elem->n_sides(); ++s)
  //   {
  //     FEInterface::dofs_on_side(_current_elem, _dim, _fe_type, s, _side_dofs);

  //     // Some side dofs are on nodes/edges and already
  //     // fixed, others are free to calculate
  //     _free_dofs = 0;
  //     for (unsigned int i = 0; i != _side_dofs.size(); ++i)
  //       if (!_dof_is_fixed[_side_dofs[i]])
  //         _free_dof[_free_dofs++] = i;

  //     // There may be nothing to project
  //     if (!_free_dofs)
  //       continue;

  //     // Initialize FE data on the side
  //     fe->attach_quadrature_rule(qsiderule.get());
  //     fe->reinit(_current_elem, s);
  //     _n_qp = qsiderule->n_points();

  //     choleskySolve(false);
  //   }

  // // Project the interior values, finally

  // // Some interior dofs are on nodes/edges/sides and
  // // already fixed, others are free to calculate
  // _free_dofs = 0;
  // for (unsigned int i = 0; i != n_dofs; ++i)
  //   if (!_dof_is_fixed[i])
  //     _free_dof[_free_dofs++] = i;

  // // There may be nothing to project
  // if (_free_dofs)
  // {
  //   // Initialize FE data
  //   fe->attach_quadrature_rule(qrule.get());
  //   fe->reinit(_current_elem);
  //   _n_qp = qrule->n_points();

  //   choleskySolve(true);
  // } // if there are free interior dofs

  // // Make sure every DoF got reached!
  // for (unsigned int i = 0; i != n_dofs; ++i)
  //   libmesh_assert(_dof_is_fixed[i]);

  // // Lock the new_vector since it is shared among threads.
  // {
  //   Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  //   for (size_t i = 0; i < mask.size(); i++)
  //     if (mask(i))
  //       _var.setDofValue(_Ue(i), i);
  // }
}

template class FVInitialConditionTempl<Real>;
template class FVInitialConditionTempl<RealVectorValue>;
template class FVInitialConditionTempl<RealEigenVector>;
