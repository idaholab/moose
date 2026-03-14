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

#include <algorithm>
#include <cmath>
#include <limits>

ComputeLinearFVLimitedGradientThread::ComputeLinearFVLimitedGradientThread(
    FEProblemBase & fe_problem,
    SystemBase & system,
    const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_gradient,
    std::vector<std::unique_ptr<NumericVector<Number>>> & temporary_limited_gradient,
    const Moose::FV::GradientLimiterType limiter_type)
  : _fe_problem(fe_problem),
    _dim(_fe_problem.mesh().dimension()),
    _system(system),
    _libmesh_system(system.system()),
    _system_number(_libmesh_system.number()),
    _raw_gradient(raw_gradient),
    _limiter_type(limiter_type),
    _temporary_limited_gradient(temporary_limited_gradient)
{
}

ComputeLinearFVLimitedGradientThread::ComputeLinearFVLimitedGradientThread(
    ComputeLinearFVLimitedGradientThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _dim(x._dim),
    _system(x._system),
    _libmesh_system(x._libmesh_system),
    _system_number(x._system_number),
    _raw_gradient(x._raw_gradient),
    _limiter_type(x._limiter_type),
    _temporary_limited_gradient(x._temporary_limited_gradient)
{
}

void
ComputeLinearFVLimitedGradientThread::operator()(const ElemInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  const auto & raw_grad_container = _raw_gradient;

  if (_limiter_type != Moose::FV::GradientLimiterType::Venkatakrishnan)
    mooseError("ComputeLinearFVLimitedGradientThread currently supports only the Venkatakrishnan "
               "limiter.");

  unsigned int size = 0;

  for (const auto & variable : _system.getVariables(_tid))
  {
    _current_var = dynamic_cast<MooseLinearVariableFV<Real> *>(variable);
    if (!_current_var)
      continue;

    if (!_current_var->needsGradientVectorStorage())
      continue;

    if (!size)
      size = range.size();

    std::vector<std::vector<Real>> temporary_values(_temporary_limited_gradient.size(),
                                                    std::vector<Real>(size, 0.0));
    std::vector<dof_id_type> dof_indices(size, 0);

    PetscVectorReader solution_reader(*_libmesh_system.current_local_solution);
    std::vector<PetscVectorReader> grad_reader;
    grad_reader.reserve(raw_grad_container.size());
    for (const auto dim_index : index_range(raw_grad_container))
      grad_reader.emplace_back(*raw_grad_container[dim_index]);

    mooseAssert(raw_grad_container.size() >= _dim,
                "Raw gradient container has fewer components than mesh dimension.");
    mooseAssert(_temporary_limited_gradient.size() >= _dim,
                "Limited gradient container has fewer components than mesh dimension.");

    auto elem_iterator = range.begin();
    for (const auto elem_i : make_range(size))
    {
      const auto & elem_info = *elem_iterator;
      elem_iterator++;

      if (!_current_var->hasBlocks(elem_info->subdomain_id()))
        continue;

      const dof_id_type dof = elem_info->dofIndices()[_system_number][_current_var->number()];
      if (dof == libMesh::DofObject::invalid_id)
        continue;

      dof_indices[elem_i] = dof;

      const Real phi_elem = solution_reader(dof);
      Real max_value = phi_elem;
      Real min_value = phi_elem;

      // Gather one-ring min/max values.
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

      // Read the raw cell gradient.
      VectorValue<Real> raw_grad;
      raw_grad.zero();
      for (const auto dim_index : make_range(_dim))
        raw_grad(dim_index) = grad_reader[dim_index](dof);

      // If the stencil is constant (or nearly constant), don't attempt to limit.
      if (std::abs(max_value - min_value) < 1e-14)
      {
        for (const auto dim_index : make_range(_dim))
          temporary_values[dim_index][elem_i] = raw_grad(dim_index);
        continue;
      }

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

        const Point face_point = fi->faceCentroid() - fi->skewnessCorrectionVector();

        const Real delta_face = raw_grad * (face_point - elem_centroid);

        Real h = elem->hmin();
        Real grad_mag = raw_grad.norm();

        Real eps = 0.1 * (grad_mag * h) * (grad_mag * h) + 1e-20;

        const Real delta_max = std::abs(max_value - phi_elem) + eps;
        const Real delta_min = std::abs(min_value - phi_elem) + eps;

        const Real rf = (delta_face >= 0.0) ? std::abs(delta_face) / delta_max
                                            : std::abs(delta_face) / delta_min;

        const Real beta = (2.0 * rf + 1.0) / (rf * (2.0 * rf + 1.0) + 1.0);
        alpha = std::min(alpha, beta);
      }

      const VectorValue<Real> limited_grad = alpha * raw_grad;
      for (const auto dim_index : make_range(_dim))
        temporary_values[dim_index][elem_i] = limited_grad(dim_index);
    }

    for (const auto dim_index : make_range(_dim))
    {
      _temporary_limited_gradient[dim_index]->add_vector(temporary_values[dim_index].data(),
                                                         dof_indices);
    }
  }
}

void
ComputeLinearFVLimitedGradientThread::join(const ComputeLinearFVLimitedGradientThread & /*y*/)
{
}
