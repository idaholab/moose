//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLinearFVGreenGaussGradientVolumeThread.h"
#include "LinearSystem.h"
#include "LinearFVBoundaryCondition.h"
#include "PetscVectorReader.h"

ComputeLinearFVGreenGaussGradientVolumeThread::ComputeLinearFVGreenGaussGradientVolumeThread(
    FEProblemBase & fe_problem, const unsigned int linear_system_num)
  : _fe_problem(fe_problem),
    _dim(_fe_problem.mesh().dimension()),
    _linear_system_number(linear_system_num),
    _linear_system(libMesh::cast_ref<libMesh::LinearImplicitSystem &>(
        _fe_problem.getLinearSystem(_linear_system_number).system())),
    _system_number(_linear_system.number())
{
}

ComputeLinearFVGreenGaussGradientVolumeThread::ComputeLinearFVGreenGaussGradientVolumeThread(
    ComputeLinearFVGreenGaussGradientVolumeThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _dim(x._dim),
    _linear_system_number(x._linear_system_number),
    _linear_system(x._linear_system),
    _system_number(x._system_number)
{
}

void
ComputeLinearFVGreenGaussGradientVolumeThread::operator()(const ElemInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  auto & linear_system = _fe_problem.getLinearSystem(_linear_system_number);

  // This will be the vector we work on since the old gradient might still be needed
  // to compute extrapolated boundary conditions for example.
  auto & grad_container = linear_system.newGradientContainer();

  // Computing this size can be very expensive so we only want to do it once
  unsigned int size = 0;

  for (const auto & variable : linear_system.getVariables(_tid))
  {
    _current_var = dynamic_cast<MooseLinearVariableFV<Real> *>(variable);
    mooseAssert(_current_var,
                "This should be a linear FV variable, did we somehow add a nonlinear variable to "
                "the linear system?");
    if (_current_var->needsGradientVectorStorage())
    {
      if (!size)
        size = range.size();

      const auto rz_radial_coord = _fe_problem.mesh().getAxisymmetricRadialCoord();
      const auto state = Moose::currentState();

      std::vector<std::vector<Real>> new_values(grad_container.size(),
                                                std::vector<Real>(size, 0.0));
      std::vector<dof_id_type> dof_indices(size, 0);
      {
        std::vector<PetscVectorReader> grad_reader;
        for (const auto dim_index : index_range(grad_container))
          grad_reader.emplace_back(*linear_system.newGradientContainer()[dim_index]);

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

            for (const auto dim_index : index_range(grad_container))
              new_values[dim_index][elem_i] = grad_reader[dim_index](dof_indices[elem_i]) / volume;

            if (coord_type == Moose::CoordinateSystemType::COORD_RZ)
            {
              const auto radial_contrib = _current_var->getElemValue(*elem_info, state) /
                                          elem_info->centroid()(rz_radial_coord);
              new_values[rz_radial_coord][elem_i] += radial_contrib;
            }
          }
          elem_iterator++;
        }
      }
      for (const auto dim_index : index_range(grad_container))
        grad_container[dim_index]->insert(new_values[dim_index].data(), dof_indices);
    }
  }
}

void
ComputeLinearFVGreenGaussGradientVolumeThread::join(
    const ComputeLinearFVGreenGaussGradientVolumeThread & /*y*/)
{
}
