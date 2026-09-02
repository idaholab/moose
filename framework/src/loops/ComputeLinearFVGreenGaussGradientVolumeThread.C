//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLinearFVGreenGaussGradientVolumeThread.h"
#include "LinearFVBoundaryCondition.h"
#include "SystemBase.h"
#include "PetscVectorReader.h"
#include "FEProblemBase.h"

ComputeLinearFVGreenGaussGradientVolumeThread::ComputeLinearFVGreenGaussGradientVolumeThread(
    FEProblemBase & fe_problem,
    SystemBase & system,
    std::vector<std::unique_ptr<NumericVector<Number>>> & gradient,
    const std::unordered_set<unsigned int> & gradient_variables)
  : _fe_problem(fe_problem),
    _dim(_fe_problem.mesh().dimension()),
    _system(system),
    _libmesh_system(system.system()),
    _system_number(_libmesh_system.number()),
    _gradient(gradient),
    _gradient_data(gradient.size()),
    _owns_gradient_data(true),
    _gradient_variables(gradient_variables)
{
  for (const auto dim_index : index_range(_gradient))
  {
    auto & gradient_vector =
        libMesh::cast_ref<libMesh::PetscVector<Number> &>(*_gradient[dim_index]);
    _gradient_data[dim_index] = gradient_vector.get_array();
  }
}

ComputeLinearFVGreenGaussGradientVolumeThread::ComputeLinearFVGreenGaussGradientVolumeThread(
    ComputeLinearFVGreenGaussGradientVolumeThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _dim(x._dim),
    _system(x._system),
    _libmesh_system(x._libmesh_system),
    _system_number(x._system_number),
    _gradient(x._gradient),
    _gradient_data(x._gradient_data),
    _owns_gradient_data(false),
    _gradient_variables(x._gradient_variables)
{
}

ComputeLinearFVGreenGaussGradientVolumeThread::~ComputeLinearFVGreenGaussGradientVolumeThread()
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
ComputeLinearFVGreenGaussGradientVolumeThread::operator()(const ElemInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  for (const auto & variable : _system.getVariables(_tid))
  {
    _current_var = dynamic_cast<MooseLinearVariableFV<Real> *>(variable);
    if (!_current_var)
      continue;

    if (!_gradient_variables.count(_current_var->number()))
      continue;

    const auto rz_radial_coord = _fe_problem.mesh().getAxisymmetricRadialCoord();
    const auto state = Moose::currentState();

    // All component vectors are clones of the same system vector and therefore
    // have the same local indexing.
    auto & first_gradient_vector =
        libMesh::cast_ref<libMesh::PetscVector<Number> &>(*_gradient.front());

    for (auto elem_iterator = range.begin(); elem_iterator != range.end(); ++elem_iterator)
    {
      const auto & elem_info = *elem_iterator;

      if (!_current_var->hasBlocks(elem_info->subdomain_id()))
        continue;

      const auto coord_type = _fe_problem.mesh().getCoordSystem(elem_info->subdomain_id());

      mooseAssert(coord_type != Moose::CoordinateSystemType::COORD_RSPHERICAL,
                  "We have not yet implemented the correct translation from gradient to "
                  "divergence for spherical coordinates yet.");

      const dof_id_type dof = elem_info->dofIndices()[_system_number][_current_var->number()];
      const auto local_dof = first_gradient_vector.map_global_to_local_index(dof);

      const auto volume = elem_info->volume() * elem_info->coordFactor();

      for (const auto dim_index : index_range(_gradient))
        _gradient_data[dim_index][local_dof] /= volume;

      if (coord_type == Moose::CoordinateSystemType::COORD_RZ)
      {
        mooseAssert(elem_info->centroid()(rz_radial_coord) != 0,
                    "Axisymmetric control volumes should not have a zero radial coordinate");

        _gradient_data[rz_radial_coord][local_dof] -=
            _current_var->getElemValue(*elem_info, state) / elem_info->centroid()(rz_radial_coord);
      }
    }
  }
}

void
ComputeLinearFVGreenGaussGradientVolumeThread::join(
    const ComputeLinearFVGreenGaussGradientVolumeThread & /*y*/)
{
}
