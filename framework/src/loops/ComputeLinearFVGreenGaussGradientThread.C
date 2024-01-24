//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLinearFVGreenGaussGradientThread.h"
#include "LinearSystem.h"
#include "LinearFVBoundaryCondition.h"

ComputeLinearFVGreenGaussGradientThread::ComputeLinearFVGreenGaussGradientThread(
    FEProblemBase & fe_problem, const unsigned int linear_system_num)
  : _fe_problem(fe_problem),
    _dim(_fe_problem.mesh().dimension()),
    _linear_system_number(linear_system_num),
    _linear_system(libMesh::cast_ref<libMesh::LinearImplicitSystem &>(
        _fe_problem.getLinearSystem(_linear_system_number).system()))
{
}

// Splitting Constructor
ComputeLinearFVGreenGaussGradientThread::ComputeLinearFVGreenGaussGradientThread(
    ComputeLinearFVGreenGaussGradientThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _dim(x._dim),
    _linear_system_number(x._linear_system_number),
    _linear_system(x._linear_system)
{
}

void
ComputeLinearFVGreenGaussGradientThread::operator()(const FaceInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  for (const auto & variable :
       _fe_problem.getLinearSystem(_linear_system_number).getVariables(_tid))
  {
    _current_var = dynamic_cast<MooseLinearVariableFV<Real> *>(variable);
    if (_current_var->needsCellGradients())
    {
      auto & grad_container = _current_var->gradientContainer();
      _new_gradient.clear();
      for (auto & vec : grad_container)
        _new_gradient.push_back(vec->zero_clone());

      mooseAssert(_current_var,
                  "This should be a linear FV variable, did we somehow add a nonlinear variable to "
                  "the linear system?");
      // Iterate over all the elements in the range
      for (const auto & face_info : range)
      {
        const auto current_face_type =
            face_info->faceType(std::make_pair(_current_var->number(), _linear_system.number()));

        if (current_face_type == FaceInfo::VarFaceNeighbors::BOTH)
          onInternalFace(*face_info);
        else if (current_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
                 current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
          onBoundaryFace(*face_info);
      }

      for (const auto i : index_range(grad_container))
      {
        _new_gradient[i]->close();
        grad_container[i] = std::move(_new_gradient[i]);
      }
    }
  }
}

void
ComputeLinearFVGreenGaussGradientThread::onInternalFace(const FaceInfo & face_info)
{
  const auto dof_id_elem =
      face_info.elemInfo()->dofIndices()[_linear_system.number()][_current_var->number()];
  const auto dof_id_neighbor =
      face_info.neighborInfo()->dofIndices()[_linear_system.number()][_current_var->number()];

  const auto & solution = *_linear_system.solution;
  const auto face_value = Moose::FV::linearInterpolation(
      solution(dof_id_elem), solution(dof_id_neighbor), face_info, true);

  const auto contribution =
      face_info.normal() * face_info.faceArea() * face_info.faceCoord() * face_value;

  for (const auto i : make_range(_dim))
  {
    _new_gradient[i]->add(dof_id_elem, contribution(i) / face_info.elemInfo()->volume());
    _new_gradient[i]->add(dof_id_neighbor, -contribution(i) / face_info.neighborInfo()->volume());
  }
}

void
ComputeLinearFVGreenGaussGradientThread::onBoundaryFace(const FaceInfo & face_info)
{
  if (auto * bc_pointer = _current_var->getBoundaryCondition(*face_info.boundaryIDs().begin()))
  {
    const auto face_type =
        face_info.faceType(std::make_pair(_current_var->number(), _linear_system.number()));
    bc_pointer->setCurrentFaceInfo(&face_info, face_type);

    dof_id_type dof_id;
    Real volume;
    if (face_info.neighborInfo())
    {
      if (face_type == FaceInfo::VarFaceNeighbors::ELEM)
      {
        dof_id =
            face_info.elemInfo()->dofIndices()[_linear_system.number()][_current_var->number()];
        volume = face_info.elemInfo()->volume();
      }
      else if (face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
      {
        dof_id =
            face_info.neighborInfo()->dofIndices()[_linear_system.number()][_current_var->number()];
        volume = face_info.neighborInfo()->volume();
      }
    }
    else
    {
      dof_id = face_info.elemInfo()->dofIndices()[_linear_system.number()][_current_var->number()];
      volume = face_info.elemInfo()->volume();
    }

    const auto contribution = face_info.normal() * face_info.faceArea() * face_info.faceCoord() *
                              bc_pointer->computeBoundaryValue();

    for (const auto i : make_range(_dim))
      _new_gradient[i]->add(dof_id, contribution(i) / volume);
  }
}

void
ComputeLinearFVGreenGaussGradientThread::join(const ComputeLinearFVGreenGaussGradientThread & /*y*/)
{
}
