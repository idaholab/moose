//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLinearFVLimitedGradientThread.h"

#include "GradientLimiterType.h"
#include "SystemBase.h"
#include "PetscVectorReader.h"
#include "FEProblemBase.h"
#include "FVUtils.h"

#include "libmesh/dof_object.h"
#include "libmesh/petsc_vector.h"

#include <algorithm>
#include <cmath>
#include <limits>

ComputeLinearFVLimitedGradientThread::ComputeLinearFVLimitedGradientThread(
    FEProblemBase & fe_problem,
    SystemBase & system,
    std::vector<std::unique_ptr<NumericVector<Number>>> & gradient,
    const Moose::FV::GradientLimiterType limiter_type,
    const std::unordered_set<unsigned int> & requested_variables)
  : _fe_problem(fe_problem),
    _dim(_fe_problem.mesh().dimension()),
    _system(system),
    _libmesh_system(system.system()),
    _system_number(_libmesh_system.number()),
    _gradient(gradient),
    _gradient_data(gradient.size()),
    _owns_gradient_data(true),
    _limiter_type(limiter_type),
    _requested_variables(requested_variables)
{
  for (const auto dim_index : index_range(_gradient))
  {
    auto & gradient_vector =
        libMesh::cast_ref<libMesh::PetscVector<Number> &>(*_gradient[dim_index]);
    _gradient_data[dim_index] = gradient_vector.get_array();
  }
}

ComputeLinearFVLimitedGradientThread::ComputeLinearFVLimitedGradientThread(
    ComputeLinearFVLimitedGradientThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _dim(x._dim),
    _system(x._system),
    _libmesh_system(x._libmesh_system),
    _system_number(x._system_number),
    _gradient(x._gradient),
    _gradient_data(x._gradient_data),
    _owns_gradient_data(false),
    _limiter_type(x._limiter_type),
    _requested_variables(x._requested_variables)
{
}

ComputeLinearFVLimitedGradientThread::~ComputeLinearFVLimitedGradientThread()
{
  if (_owns_gradient_data)
    for (const auto dim_index : index_range(_gradient))
    {
      auto & gradient_vector =
          libMesh::cast_ref<libMesh::PetscVector<Number> &>(*_gradient[dim_index]);
      gradient_vector.restore_array();
    }
}

void
ComputeLinearFVLimitedGradientThread::operator()(const ElemInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  if (_limiter_type != Moose::FV::GradientLimiterType::Venkatakrishnan)
    mooseError("ComputeLinearFVLimitedGradientThread currently supports only the Venkatakrishnan "
               "limiter.");

  mooseAssert(_gradient.size() >= _dim,
              "Gradient container has fewer components than mesh dimension.");

  // All gradient component vectors have the same layout because they are
  // clones of the same system vector.
  auto & first_gradient_vector =
      libMesh::cast_ref<libMesh::PetscVector<Number> &>(*_gradient.front());

  for (const auto & variable : _system.getVariables(_tid))
  {
    _current_var = dynamic_cast<MooseLinearVariableFV<Real> *>(variable);
    if (!_current_var)
      continue;

    if (!_current_var->needsGradientVectorStorage())
      continue;

    if (!_requested_variables.count(_current_var->number()))
      continue;

    PetscVectorReader solution_reader(*_libmesh_system.current_local_solution);

    for (auto elem_iterator = range.begin(); elem_iterator != range.end(); ++elem_iterator)
    {
      const auto & elem_info = *elem_iterator;

      if (!_current_var->hasBlocks(elem_info->subdomain_id()))
        continue;

      const dof_id_type dof = elem_info->dofIndices()[_system_number][_current_var->number()];
      if (dof == libMesh::DofObject::invalid_id)
        continue;

      const auto local_dof = first_gradient_vector.map_global_to_local_index(dof);

      const Real phi_elem = solution_reader(dof);
      Real max_value = phi_elem;
      Real min_value = phi_elem;

      // Gather one-ring min/max solution values.
      const Elem * const elem = elem_info->elem();
      for (const auto side : make_range(elem->n_sides()))
      {
        const Elem * const neighbor = elem->neighbor_ptr(side);
        if (!neighbor)
          continue;

        const auto & neighbor_info = _fe_problem.mesh().elemInfo(neighbor->id());
        if (!_current_var->hasBlocks(neighbor_info.subdomain_id()))
          continue;

        const dof_id_type neighbor_dof =
            neighbor_info.dofIndices()[_system_number][_current_var->number()];
        if (neighbor_dof == libMesh::DofObject::invalid_id)
          continue;

        const Real phi_neighbor = solution_reader(neighbor_dof);
        max_value = std::max(max_value, phi_neighbor);
        min_value = std::min(min_value, phi_neighbor);
      }

      // Copy this cell's raw gradient before modifying the gradient storage.
      VectorValue<Real> raw_grad;
      raw_grad.zero();
      for (const auto dim_index : make_range(_dim))
        raw_grad(dim_index) = _gradient_data[dim_index][local_dof];

      // If the stencil is constant (or nearly constant), leave the raw
      // gradient unchanged.
      if (std::abs(max_value - min_value) < 1e-14)
        continue;

      Real alpha = 1.0;
      const Point & elem_centroid = elem_info->centroid();

      for (const auto side : make_range(elem->n_sides()))
      {
        const Elem * const neighbor = elem->neighbor_ptr(side);
        if (!neighbor)
          continue;

        const auto & neighbor_info = _fe_problem.mesh().elemInfo(neighbor->id());
        if (!_current_var->hasBlocks(neighbor_info.subdomain_id()))
          continue;

        const dof_id_type neighbor_dof =
            neighbor_info.dofIndices()[_system_number][_current_var->number()];
        if (neighbor_dof == libMesh::DofObject::invalid_id)
          continue;

        const bool elem_has_face_info = Moose::FV::elemHasFaceInfo(*elem, neighbor);
        const Elem * const fi_elem = elem_has_face_info ? elem : neighbor;
        const unsigned int fi_side =
            elem_has_face_info ? side : neighbor->which_neighbor_am_i(elem);

        const auto * fi = _fe_problem.mesh().faceInfo(fi_elem, fi_side);
        mooseAssert(fi,
                    "Missing FaceInfo for neighboring elements with centroid " +
                        Moose::stringify(elem_info->centroid()) + " and " +
                        Moose::stringify(neighbor->vertex_average()) +
                        " while computing limited gradients.");

        const Point face_point = fi->faceCentroid();
        const Real delta_face = raw_grad * (face_point - elem_centroid);

        const Real h = elem->hmin();
        const Real grad_mag = raw_grad.norm();

        const Real eps = 0.1 * (grad_mag * h) * (grad_mag * h) + 1e-20;

        const Real delta_max = std::abs(max_value - phi_elem) + eps;
        const Real delta_min = std::abs(min_value - phi_elem) + eps;

        const Real rf = (delta_face >= 0.0) ? std::abs(delta_face) / delta_max
                                            : std::abs(delta_face) / delta_min;

        const Real beta = (2.0 * rf + 1.0) / (rf * (2.0 * rf + 1.0) + 1.0);
        alpha = std::min(alpha, beta);
      }

      // No neighboring gradient values are needed, so it is safe to replace
      // this cell's raw gradient once its limiter coefficient is known.
      for (const auto dim_index : make_range(_dim))
        _gradient_data[dim_index][local_dof] = alpha * raw_grad(dim_index);
    }
  }
}

void
ComputeLinearFVLimitedGradientThread::join(const ComputeLinearFVLimitedGradientThread & /*y*/)
{
}
