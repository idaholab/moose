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
    FEProblemBase & fe_problem,
    const unsigned int linear_system_num,
    MooseLinearVariableFV<Real> * var)
  : _fe_problem(fe_problem),
    _dim(_fe_problem.mesh().dimension()),
    _linear_system_number(linear_system_num),
    _linear_system(libMesh::cast_ref<libMesh::LinearImplicitSystem &>(
        _fe_problem.getLinearSystem(_linear_system_number).system())),
    _var(var)
{
}

// Splitting Constructor
ComputeLinearFVGreenGaussGradientThread::ComputeLinearFVGreenGaussGradientThread(
    ComputeLinearFVGreenGaussGradientThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _dim(x._dim),
    _linear_system_number(x._linear_system_number),
    _linear_system(x._linear_system),
    _var(x._var)
{
}

void
ComputeLinearFVGreenGaussGradientThread::operator()(const FaceInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  // Iterate over all the elements in the range
  for (const auto & face_info : range)
  {
    const auto current_face_type =
        face_info->faceType(std::make_pair(_var->number(), _var->sys().number()));

    if (current_face_type == FaceInfo::VarFaceNeighbors::BOTH)
      onInternalFace(*face_info);
    else if (current_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
             current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
      onBoundaryFace(*face_info);
  }
}

void
ComputeLinearFVGreenGaussGradientThread::onInternalFace(const FaceInfo & face_info)
{
  const auto dof_id_elem = face_info.elemInfo()->dofIndices()[_var->sys().number()][_var->number()];
  const auto dof_id_neighbor =
      face_info.neighborInfo()->dofIndices()[_var->sys().number()][_var->number()];

  const auto & solution = *_linear_system.solution;
  const auto face_value = Moose::FV::linearInterpolation(
      solution(dof_id_elem), solution(dof_id_neighbor), face_info, true);

  const auto contribution =
      face_info.normal() * face_info.faceArea() * face_info.faceCoord() * face_value;

  const auto & grad_container = _var->gradientContainer();
  for (const auto i : make_range(_dim))
  {
    grad_container[i]->add(dof_id_elem, contribution(i));
    grad_container[i]->add(dof_id_neighbor, -contribution(i));
  }
}

void
ComputeLinearFVGreenGaussGradientThread::onBoundaryFace(const FaceInfo & face_info)
{
  if (auto * bc_pointer = _var->getBoundaryCondition(*face_info.boundaryIDs().begin()))
  {
    const auto face_type = face_info.faceType(std::make_pair(_var->number(), _var->sys().number()));
    bc_pointer->setCurrentFaceInfo(&face_info, face_type);

    dof_id_type dof_id;
    if (face_info.neighborInfo())
    {
      if (face_type == FaceInfo::VarFaceNeighbors::ELEM)
        dof_id = face_info.elemInfo()->dofIndices()[_var->sys().number()][_var->number()];
      else if (face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
        dof_id = face_info.neighborInfo()->dofIndices()[_var->sys().number()][_var->number()];
    }
    else
      dof_id = face_info.elemInfo()->dofIndices()[_var->sys().number()][_var->number()];

    const auto contribution = face_info.normal() * face_info.faceArea() * face_info.faceCoord() *
                              bc_pointer->computeBoundaryValue();

    const auto & grad_container = _var->gradientContainer();
    for (const auto i : make_range(_dim))
      grad_container[i]->add(dof_id, contribution(i));
  }
}

void
ComputeLinearFVGreenGaussGradientThread::join(const ComputeLinearFVGreenGaussGradientThread & /*y*/)
{
}
