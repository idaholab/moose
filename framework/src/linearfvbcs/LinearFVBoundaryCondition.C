//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVBoundaryCondition.h"
#include "Problem.h"
#include "SystemBase.h"
#include "MooseVariableFV.h"

InputParameters
LinearFVBoundaryCondition::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += TransientInterface::validParams();
  params += BoundaryRestrictableRequired::validParams();
  params += TaggingInterface::validParams();
  params += NonADFunctorInterface::validParams();

  MultiMooseEnum vtags("rhs time", "rhs", true);
  auto & vector_tag_enum = params.set<MultiMooseEnum>("vector_tags", true);
  vector_tag_enum = vtags;

  params.addRequiredParam<LinearVariableName>(
      "variable", "The name of the variable that this boundary condition applies to");
  params.declareControllable("enable");
  params.registerBase("LinearFVBoundaryCondition");
  params.registerSystemAttributeName("LinearFVBoundaryCondition");
  return params;
}

LinearFVBoundaryCondition::LinearFVBoundaryCondition(const InputParameters & parameters)
  : MooseObject(parameters),
    BoundaryRestrictableRequired(this, false),
    SetupInterface(this),
    FunctionInterface(this),
    DistributionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this),
    GeometricSearchInterface(this),
    MeshChangedInterface(parameters),
    TaggingInterface(this),
    MooseVariableInterface(this,
                           false,
                           "variable",
                           Moose::VarKindType::VAR_SOLVER,
                           Moose::VarFieldType::VAR_FIELD_STANDARD),
    MooseVariableDependencyInterface(this),
    NonADFunctorInterface(this),
    FaceArgProducerInterface(),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _mesh(_subproblem.mesh()),
    _fv_problem(*getCheckedPointerParam<FVProblemBase *>("_fe_problem_base")),
    _var(*mooseLinearVariableFV()),
    _sys(_var.sys()),
    _linear_system(libMesh::cast_ref<libMesh::LinearImplicitSystem &>(_sys.system())),
    _var_num(_var.number()),
    _sys_num(_sys.number())
{
  addMooseVariableDependency(&_var);
}

bool
LinearFVBoundaryCondition::hasFaceSide(const FaceInfo & fi, bool fi_elem_side) const
{
  const auto ft = fi.faceType(std::make_pair(_var_num, _sys_num));
  if (fi_elem_side)
    return ft == FaceInfo::VarFaceNeighbors::ELEM || ft == FaceInfo::VarFaceNeighbors::BOTH;
  else
    return ft == FaceInfo::VarFaceNeighbors::NEIGHBOR || ft == FaceInfo::VarFaceNeighbors::BOTH;
}

Moose::FaceArg
LinearFVBoundaryCondition::singleSidedFaceArg(const FaceInfo * fi,
                                              const Moose::FV::LimiterType limiter_type,
                                              const bool correct_skewness) const
{
  mooseAssert(fi, "FaceInfo should not be null!");
  return makeFace(*fi, limiter_type, true, correct_skewness);
}

Real
LinearFVBoundaryCondition::computeCellToFaceDistance() const
{
  const auto cell_to_face_vector = computeCellToFaceVector();
  return std::abs(cell_to_face_vector * _current_face_info->normal());
}

RealVectorValue
LinearFVBoundaryCondition::computeCellToFaceVector() const
{
  const auto is_on_mesh_boundary = !_current_face_info->neighborPtr();
  const auto defined_on_elem =
      is_on_mesh_boundary ? true : (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM);
  if (is_on_mesh_boundary)
    return _current_face_info->dCN();
  else
    return (_current_face_info->faceCentroid() - (defined_on_elem
                                                      ? _current_face_info->elemCentroid()
                                                      : _current_face_info->neighborCentroid()));
}
