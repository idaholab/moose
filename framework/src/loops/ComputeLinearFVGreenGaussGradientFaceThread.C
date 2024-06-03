//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLinearFVGreenGaussGradientFaceThread.h"
#include "LinearSystem.h"
#include "LinearFVBoundaryCondition.h"
#include "PetscVectorReader.h"

ComputeLinearFVGreenGaussGradientFaceThread::ComputeLinearFVGreenGaussGradientFaceThread(
    FEProblemBase & fe_problem, const unsigned int linear_system_num)
  : _fe_problem(fe_problem),
    _dim(_fe_problem.mesh().dimension()),
    _linear_system_number(linear_system_num),
    _linear_system(libMesh::cast_ref<libMesh::LinearImplicitSystem &>(
        _fe_problem.getLinearSystem(_linear_system_number).system())),
    _system_number(_linear_system.number()),
    _new_gradient(_fe_problem.getLinearSystem(_linear_system_number).newGradientContainer())
{
}

ComputeLinearFVGreenGaussGradientFaceThread::ComputeLinearFVGreenGaussGradientFaceThread(
    ComputeLinearFVGreenGaussGradientFaceThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _dim(x._dim),
    _linear_system_number(x._linear_system_number),
    _linear_system(x._linear_system),
    _system_number(x._system_number),
    // This will be the vector we work on since the old gradient might still be needed
    // to compute extrapolated boundary conditions for example.
    _new_gradient(x._new_gradient)
{
}

void
ComputeLinearFVGreenGaussGradientFaceThread::operator()(const FaceInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  unsigned int size = 0;

  for (const auto & variable :
       _fe_problem.getLinearSystem(_linear_system_number).getVariables(_tid))
  {
    _current_var = dynamic_cast<MooseLinearVariableFV<Real> *>(variable);
    mooseAssert(_current_var,
                "This should be a linear FV variable, did we somehow add a nonlinear variable to "
                "the linear system?");
    if (_current_var->needsGradientVectorStorage())
    {
      if (!size)
        size = range.size();

      std::vector<std::vector<Real>> new_values_elem(_new_gradient.size(),
                                                     std::vector<Real>(size, 0.0));
      std::vector<std::vector<Real>> new_values_neighbor(_new_gradient.size(),
                                                         std::vector<Real>(size, 0.0));
      std::vector<dof_id_type> dof_indices_elem(size, 0);
      std::vector<dof_id_type> dof_indices_neighbor(size, 0);

      {
        PetscVectorReader solution_reader(*_linear_system.current_local_solution);

        // Iterate over all the face infos in the range
        auto face_iterator = range.begin();
        for (const auto & face_i : make_range(size))
        {
          const auto & face_info = *face_iterator;

          const auto current_face_type =
              face_info->faceType(std::make_pair(_current_var->number(), _system_number));

          // First we check if this face is internal to the variable, if yes, contribute to both
          // sides
          if (current_face_type == FaceInfo::VarFaceNeighbors::BOTH)
          {
            dof_indices_elem[face_i] =
                face_info->elemInfo()->dofIndices()[_system_number][_current_var->number()];
            dof_indices_neighbor[face_i] =
                face_info->neighborInfo()->dofIndices()[_system_number][_current_var->number()];

            const auto face_value =
                Moose::FV::linearInterpolation(solution_reader(dof_indices_elem[face_i]),
                                               solution_reader(dof_indices_neighbor[face_i]),
                                               *face_info,
                                               true);

            const auto contribution =
                face_info->normal() * face_info->faceArea() * face_info->faceCoord() * face_value;

            for (const auto i : make_range(_dim))
            {
              new_values_elem[i][face_i] = contribution(i);
              new_values_neighbor[i][face_i] = -contribution(i);
            }
          }
          // If this face is on the boundary of the block where the variable is defined, we
          // check for boundary conditions. If we don't find any we use an automatic one-term
          // expansion to compute the face value.
          else if (current_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
                   current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
          {
            auto * bc_pointer =
                _current_var->getBoundaryCondition(*face_info->boundaryIDs().begin());

            if (bc_pointer)
              bc_pointer->setupFaceData(face_info, current_face_type);

            const auto * const elem_info = current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                               ? face_info->elemInfo()
                                               : face_info->neighborInfo();
            auto & dof_id_container = current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                          ? dof_indices_elem
                                          : dof_indices_neighbor;
            auto & contribution_container = current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                                ? new_values_elem
                                                : new_values_neighbor;

            dof_id_container[face_i] =
                elem_info->dofIndices()[_system_number][_current_var->number()];

            // If we don't have a boundary condition, then it's a natural condition. We'll use a
            // one-term expansion approximation in that case
            const auto contribution = face_info->normal() * face_info->faceArea() *
                                      face_info->faceCoord() *
                                      (bc_pointer ? bc_pointer->computeBoundaryValue()
                                                  : solution_reader(dof_id_container[face_i]));

            for (const auto i : make_range(_dim))
              contribution_container[i][face_i] = contribution(i);
          }
          face_iterator++;
        }
      }
      for (const auto i : make_range(_dim))
      {
        _new_gradient[i]->add_vector(new_values_elem[i].data(), dof_indices_elem);
        _new_gradient[i]->add_vector(new_values_neighbor[i].data(), dof_indices_neighbor);
      }
    }
  }
}

void
ComputeLinearFVGreenGaussGradientFaceThread::join(
    const ComputeLinearFVGreenGaussGradientFaceThread & /*y*/)
{
}
