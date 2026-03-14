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
    std::vector<std::unique_ptr<NumericVector<Number>>> & temporary_gradient)
  : _fe_problem(fe_problem),
    _dim(_fe_problem.mesh().dimension()),
    _system(system),
    _libmesh_system(system.system()),
    _system_number(_libmesh_system.number()),
    _temporary_gradient(temporary_gradient)
{
}

ComputeLinearFVGreenGaussGradientVolumeThread::ComputeLinearFVGreenGaussGradientVolumeThread(
    ComputeLinearFVGreenGaussGradientVolumeThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _dim(x._dim),
    _system(x._system),
    _libmesh_system(x._libmesh_system),
    _system_number(x._system_number),
    _temporary_gradient(x._temporary_gradient)
{
}

void
ComputeLinearFVGreenGaussGradientVolumeThread::operator()(const ElemInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  // Computing this size can be very expensive so we only want to do it once
  unsigned int size = 0;

  for (const auto & variable : _system.getVariables(_tid))
  {
    _current_var = dynamic_cast<MooseLinearVariableFV<Real> *>(variable);
    if (!_current_var)
      continue;

    if (_current_var->needsGradientVectorStorage())
    {
      if (!size)
        size = range.size();

      const auto rz_radial_coord = _fe_problem.mesh().getAxisymmetricRadialCoord();
      const auto state = Moose::currentState();

      std::vector<std::vector<Real>> temporary_values(_temporary_gradient.size(),
                                                      std::vector<Real>(size, 0.0));
      std::vector<dof_id_type> dof_indices(size, 0);
      {
        std::vector<PetscVectorReader> grad_reader;
        for (const auto dim_index : index_range(_temporary_gradient))
          grad_reader.emplace_back(*_temporary_gradient[dim_index]);

        // Iterate over all the elements in the range
        auto elem_iterator = range.begin();
        for (const auto elem_i : make_range(size))
        {
          const auto & elem_info = *elem_iterator;
          if (_current_var->hasBlocks(elem_info->subdomain_id()))
          {
            const auto coord_type = _fe_problem.mesh().getCoordSystem(elem_info->subdomain_id());

            mooseAssert(coord_type != Moose::CoordinateSystemType::COORD_RSPHERICAL,
                        "We have not yet implemented the correct translation from gradient to "
                        "divergence for "
                        "spherical coordinates yet.");

            dof_indices[elem_i] = elem_info->dofIndices()[_system_number][_current_var->number()];
            const auto volume = elem_info->volume() * elem_info->coordFactor();

            for (const auto dim_index : index_range(_temporary_gradient))
              temporary_values[dim_index][elem_i] =
                  grad_reader[dim_index](dof_indices[elem_i]) / volume;

            if (coord_type == Moose::CoordinateSystemType::COORD_RZ)
            {
              mooseAssert(elem_info->centroid()(rz_radial_coord) != 0,
                          "Axisymmetric control volumes should not have a zero radial coordinate");
              temporary_values[rz_radial_coord][elem_i] -=
                  _current_var->getElemValue(*elem_info, state) /
                  elem_info->centroid()(rz_radial_coord);
            }
          }
          elem_iterator++;
        }
      }
      for (const auto dim_index : index_range(_temporary_gradient))
      {
        _temporary_gradient[dim_index]->zero();
        _temporary_gradient[dim_index]->add_vector(temporary_values[dim_index].data(), dof_indices);
      }
    }
  }
}

void
ComputeLinearFVGreenGaussGradientVolumeThread::join(
    const ComputeLinearFVGreenGaussGradientVolumeThread & /*y*/)
{
}
