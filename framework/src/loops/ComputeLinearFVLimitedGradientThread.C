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
#include "LinearSystem.h"
#include "PetscVectorReader.h"
#include "FEProblemBase.h"

#include "libmesh/dof_object.h"

#include <algorithm>
#include <cmath>
#include <limits>

ComputeLinearFVLimitedGradientThread::ComputeLinearFVLimitedGradientThread(
    FEProblemBase & fe_problem,
    const unsigned int linear_system_num,
    const Moose::FV::GradientLimiterType limiter_type)
  : _fe_problem(fe_problem),
    _dim(_fe_problem.mesh().dimension()),
    _linear_system_number(linear_system_num),
    _linear_system(libMesh::cast_ref<libMesh::LinearImplicitSystem &>(
        _fe_problem.getLinearSystem(_linear_system_number).system())),
    _system_number(_linear_system.number()),
    _limiter_type(limiter_type),
    _new_limited_gradient(_fe_problem.getLinearSystem(_linear_system_number)
                              .newLimitedGradientContainer(_limiter_type))
{
}

ComputeLinearFVLimitedGradientThread::ComputeLinearFVLimitedGradientThread(
    ComputeLinearFVLimitedGradientThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _dim(x._dim),
    _linear_system_number(x._linear_system_number),
    _linear_system(x._linear_system),
    _system_number(x._system_number),
    _limiter_type(x._limiter_type),
    _new_limited_gradient(x._new_limited_gradient)
{
}

void
ComputeLinearFVLimitedGradientThread::operator()(const ElemInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  auto & linear_system = _fe_problem.getLinearSystem(_linear_system_number);
  const auto & raw_grad_container = linear_system.gradientContainer();

  if (_limiter_type != Moose::FV::GradientLimiterType::Venkatakrishnan)
    mooseError("ComputeLinearFVLimitedGradientThread currently supports only the Venkatakrishnan "
               "limiter.");

  unsigned int size = 0;

  for (const auto & variable : linear_system.getVariables(_tid))
  {
    _current_var = dynamic_cast<MooseLinearVariableFV<Real> *>(variable);
    mooseAssert(_current_var,
                "This should be a linear FV variable, did we somehow add a nonlinear variable to "
                "the linear system?");

    if (!_current_var->needsGradientVectorStorage())
      continue;

    if (!size)
      size = range.size();

    std::vector<std::vector<Real>> new_values(_new_limited_gradient.size(),
                                              std::vector<Real>(size, 0.0));
    std::vector<dof_id_type> dof_indices(size, 0);

    PetscVectorReader solution_reader(*_linear_system.current_local_solution);
    std::vector<PetscVectorReader> grad_reader;
    grad_reader.reserve(raw_grad_container.size());
    for (const auto dim_index : index_range(raw_grad_container))
      grad_reader.emplace_back(*raw_grad_container[dim_index]);

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
          new_values[dim_index][elem_i] = raw_grad(dim_index);
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

        Point face_point;
        if (const auto * const fi = _fe_problem.mesh().faceInfo(elem, side))
          face_point = fi->faceCentroid() - fi->skewnessCorrectionVector();
        else
          face_point = 0.5 * (elem_centroid + neighbor_info.centroid());

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
        new_values[dim_index][elem_i] = limited_grad(dim_index);
    }

    for (const auto dim_index : make_range(_dim))
    {
      _new_limited_gradient[dim_index]->add_vector(new_values[dim_index].data(), dof_indices);
    }
  }
}

void
ComputeLinearFVLimitedGradientThread::join(const ComputeLinearFVLimitedGradientThread & /*y*/)
{
}
