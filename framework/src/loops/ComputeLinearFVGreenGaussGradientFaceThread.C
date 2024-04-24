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

ComputeLinearFVGreenGaussGradientFaceThread::ComputeLinearFVGreenGaussGradientFaceThread(
    FEProblemBase & fe_problem, const unsigned int linear_system_num)
  : _fe_problem(fe_problem),
    _dim(_fe_problem.mesh().dimension()),
    _linear_system_number(linear_system_num),
    _linear_system(libMesh::cast_ref<libMesh::LinearImplicitSystem &>(
        _fe_problem.getLinearSystem(_linear_system_number).system())),
    _global_system_number(_linear_system.number()),
    _new_gradient(_fe_problem.getLinearSystem(_linear_system_number).newGradientContainer()),
    _dof_indices(2, 0),
    _contributions(2, 0.0)
{
}

ComputeLinearFVGreenGaussGradientFaceThread::ComputeLinearFVGreenGaussGradientFaceThread(
    ComputeLinearFVGreenGaussGradientFaceThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _dim(x._dim),
    _linear_system_number(x._linear_system_number),
    _linear_system(x._linear_system),
    _global_system_number(x._global_system_number),
    // This will be the vector we work on since the old gradient might still be needed
    // to compute extrapolated boundary conditions for example.
    _new_gradient(x._new_gradient),
    _dof_indices(x._dof_indices),
    _contributions(x._contributions)
{
}

void
ComputeLinearFVGreenGaussGradientFaceThread::operator()(const FaceInfoRange & range)
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
    if (_current_var->needsGradientVectorStorage())
      // Iterate over all the elements in the range
      for (const auto & face_info : range)
      {
        const auto current_face_type =
            face_info->faceType(std::make_pair(_current_var->number(), _global_system_number));

        if (current_face_type == FaceInfo::VarFaceNeighbors::BOTH)
          onInternalFace(*face_info);
        else if (current_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
                 current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
          onBoundaryFace(*face_info);
      }
  }
}

void
ComputeLinearFVGreenGaussGradientFaceThread::onInternalFace(const FaceInfo & face_info)
{
  _dof_indices[0] =
      face_info.elemInfo()->dofIndices()[_global_system_number][_current_var->number()];
  _dof_indices[1] =
      face_info.neighborInfo()->dofIndices()[_global_system_number][_current_var->number()];

  const auto & solution = *_linear_system.current_local_solution;
  const auto face_value = Moose::FV::linearInterpolation(
      solution(_dof_indices[0]), solution(_dof_indices[1]), face_info, true);

  const auto contribution =
      face_info.normal() * face_info.faceArea() * face_info.faceCoord() * face_value;

  for (const auto i : make_range(_dim))
  {
    _contributions[0] = contribution(i);
    _contributions[1] = -contribution(i);
    _new_gradient[i]->add_vector(_contributions.data(), _dof_indices);
  }
}

void
ComputeLinearFVGreenGaussGradientFaceThread::onBoundaryFace(const FaceInfo & face_info)
{
  auto * bc_pointer = _current_var->getBoundaryCondition(*face_info.boundaryIDs().begin());

  const auto face_type =
      face_info.faceType(std::make_pair(_current_var->number(), _global_system_number));
  if (bc_pointer)
    bc_pointer->setCurrentFaceInfo(&face_info, face_type);

  const auto * const elem_info = face_type == FaceInfo::VarFaceNeighbors::ELEM
                                     ? face_info.elemInfo()
                                     : face_info.neighborInfo();
  const auto state = Moose::currentState();

  const auto dof_id = elem_info->dofIndices()[_global_system_number][_current_var->number()];

  // If we don't have a boundary condition, then it's a natural condition. We'll use a one-term
  // expansion approximation in that case
  const auto contribution = face_info.normal() * face_info.faceArea() * face_info.faceCoord() *
                            (bc_pointer ? bc_pointer->computeBoundaryValue()
                                        : _current_var->getElemValue(*elem_info, state));

  for (const auto i : make_range(_dim))
    _new_gradient[i]->add(dof_id, contribution(i));
}

void
ComputeLinearFVGreenGaussGradientFaceThread::join(
    const ComputeLinearFVGreenGaussGradientFaceThread & /*y*/)
{
}
