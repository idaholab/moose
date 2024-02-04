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

ComputeLinearFVGreenGaussGradientVolumeThread::ComputeLinearFVGreenGaussGradientVolumeThread(
    FEProblemBase & fe_problem, const unsigned int linear_system_num)
  : _fe_problem(fe_problem),
    _dim(_fe_problem.mesh().dimension()),
    _linear_system_number(linear_system_num),
    _linear_system(libMesh::cast_ref<libMesh::LinearImplicitSystem &>(
        _fe_problem.getLinearSystem(_linear_system_number).system()))
{
}

ComputeLinearFVGreenGaussGradientVolumeThread::ComputeLinearFVGreenGaussGradientVolumeThread(
    ComputeLinearFVGreenGaussGradientVolumeThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _dim(x._dim),
    _linear_system_number(x._linear_system_number),
    _linear_system(x._linear_system)
{
}

void
ComputeLinearFVGreenGaussGradientVolumeThread::operator()(const ElemInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  for (const auto & variable :
       _fe_problem.getLinearSystem(_linear_system_number).getVariables(_tid))
  {
    _current_var = dynamic_cast<MooseLinearVariableFV<Real> *>(variable);
    mooseAssert(_current_var,
                "This should be a linear FV variable, did we somehow add a nonlinear variable to "
                "the linear system?");
    if (_current_var->needsCellGradients())
    {
      const auto rz_radial_coord = _fe_problem.mesh().getAxisymmetricRadialCoord();
      const auto state = Moose::currentState();
      auto & grad_container = _current_var->gradientContainer();
      // Iterate over all the elements in the range
      for (const auto & elem_info : range)
        if (_current_var->hasBlocks(elem_info->subdomain_id()))
        {
          const auto coord_type = _fe_problem.mesh().getCoordSystem(elem_info->subdomain_id());
          const auto dof_id_elem =
              elem_info->dofIndices()[_linear_system.number()][_current_var->number()];
          auto volume = elem_info->volume();
          if (coord_type == Moose::CoordinateSystemType::COORD_RZ)
            volume *= elem_info->coordFactor();

          for (const auto dim_index : index_range(grad_container))
            grad_container[dim_index]->set(dof_id_elem,
                                           (*grad_container[dim_index])(dof_id_elem) / volume);

          if (coord_type == Moose::CoordinateSystemType::COORD_RZ)
          {
            const auto radial_contrib = _current_var->getElemValue(*elem_info, state) /
                                        elem_info->centroid()(rz_radial_coord);
            grad_container[rz_radial_coord]->add(dof_id_elem, radial_contrib);
          }
        }
    }
  }
}

void
ComputeLinearFVGreenGaussGradientVolumeThread::join(
    const ComputeLinearFVGreenGaussGradientVolumeThread & /*y*/)
{
}
