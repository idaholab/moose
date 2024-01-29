//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseLinearVariableFV.h"
#include "TimeIntegrator.h"
#include "NonlinearSystemBase.h"
#include "DisplacedSystem.h"
#include "SystemBase.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "MathFVUtils.h"
#include "FVUtils.h"
#include "FVFluxBC.h"
#include "FVDirichletBCBase.h"
#include "GreenGaussGradient.h"
#include "LinearFVBoundaryCondition.h"

#include "libmesh/numeric_vector.h"

#include <climits>
#include <typeinfo>

using namespace Moose;

registerMooseObject("MooseApp", MooseLinearVariableFVReal);

template <typename OutputType>
InputParameters
MooseLinearVariableFV<OutputType>::validParams()
{
  InputParameters params = MooseVariableField<OutputType>::validParams();
  params.set<bool>("fv") = true;
  params.set<MooseEnum>("family") = "MONOMIAL";
  params.set<MooseEnum>("order") = "CONSTANT";
  return params;
}

template <typename OutputType>
MooseLinearVariableFV<OutputType>::MooseLinearVariableFV(const InputParameters & parameters)
  : MooseVariableField<OutputType>(parameters),
    _solution(this->_sys.currentSolution()),
    _prev_elem(nullptr),
    _needs_cell_gradients(false)
{
}

template <typename OutputType>
bool
MooseLinearVariableFV<OutputType>::isExtrapolatedBoundaryFace(
    const FaceInfo & /*fi*/, const Elem * const /*elem*/, const Moose::StateArg & /*state*/) const
{
  return false;
  // mooseAssert(!fi.boundaryIDs().empty(), "We should have a boundary face on this face!");
  // mooseAssert(this->getBoundaryCondition(*fi.boundaryIDs().begin()),
  //             "We should have the boundary conditions for the given boundary ID!");
  // return this->getBoundaryCondition(*fi.boundaryIDs().begin())->needsExtrapolation();
}

template <typename OutputType>
Moose::VarFieldType
MooseLinearVariableFV<OutputType>::fieldType() const
{
  if (std::is_same<OutputType, Real>::value)
    return Moose::VarFieldType::VAR_FIELD_STANDARD;
  else if (std::is_same<OutputType, RealVectorValue>::value)
    return Moose::VarFieldType::VAR_FIELD_VECTOR;
  else if (std::is_same<OutputType, RealEigenVector>::value)
    return Moose::VarFieldType::VAR_FIELD_ARRAY;
  else
    mooseError("Unknown variable field type");
}

template <typename OutputType>
bool
MooseLinearVariableFV<OutputType>::isArray() const
{
  return std::is_same<OutputType, RealEigenVector>::value;
}

template <typename OutputType>
bool
MooseLinearVariableFV<OutputType>::isVector() const
{
  return std::is_same<OutputType, RealVectorValue>::value;
}

template <typename OutputType>
Real
MooseLinearVariableFV<OutputType>::getElemValue(const ElemInfo * const elem_info,
                                                const StateArg & state) const
{
  mooseAssert(
      this->hasBlocks(elem_info->subdomain_id()),
      "The variable should be defined on the element's subdomain! This typically occurs when the "
      "user wants to evaluate the elements right next to the boundary of two variables (block "
      "boundary). The subdomain which is queried: " +
          Moose::stringify(this->activeSubdomains()) + " the subdomain of the element " +
          std::to_string(elem_info->subdomain_id()));

  // It's not safe to use solutionState(0) because it returns the libMesh System solution member
  // which is wrong during things like finite difference Jacobian evaluation, e.g. when PETSc
  // perturbs the solution vector we feed these perturbations into the current_local_solution
  // while the libMesh solution is frozen in the non-perturbed state
  const auto & global_soln = (state.state == 0)
                                 ? *this->_sys.currentSolution()
                                 : this->_sys.solutionState(state.state, state.iteration_type);

  return global_soln(elem_info->dofIndices()[this->_sys.number()][this->number()]);
}

template <typename OutputType>
Real
MooseLinearVariableFV<OutputType>::getBoundaryFaceValue(const FaceInfo & /*fi*/,
                                                        const StateArg & /*state*/,
                                                        const bool /*correct_skewness*/) const
{
  mooseError("Not implemented yet");
}

template <typename OutputType>
const VectorValue<Real> &
MooseLinearVariableFV<OutputType>::gradSln(const ElemInfo * const elem_info) const
{
  if (_needs_cell_gradients)
  {
    _cell_gradient.zero();
    for (const auto i : make_range(this->_mesh.dimension()))
      _cell_gradient(i) =
          (*_grad_cache[i])(elem_info->dofIndices()[this->_sys.number()][this->number()]);
  }

  return _cell_gradient;
}

template <typename OutputType>
VectorValue<Real>
MooseLinearVariableFV<OutputType>::uncorrectedAdGradSln(const FaceInfo & /*fi*/,
                                                        const StateArg & /*state*/,
                                                        const bool /*correct_skewness*/) const
{
  mooseError("Not implemented yet");
}

template <typename OutputType>
VectorValue<Real>
MooseLinearVariableFV<OutputType>::gradSln(const FaceInfo & /*fi*/) const
{
  mooseError("Not implemented yet");
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::initialSetup()
{
  MooseVariableField<OutputType>::initialSetup();

  if (_needs_cell_gradients)
  {
    _grad_cache.clear();
    for (const auto i : make_range(this->_mesh.dimension()))
    {
      (void)i;
      _grad_cache.push_back(this->_sys.currentSolution()->zero_clone());
    }
  }

  cacheBoundaryBCMap();
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::ValueType
MooseLinearVariableFV<OutputType>::evaluate(const FaceArg & face, const StateArg & state) const
{
  const FaceInfo * const fi = face.fi;

  mooseAssert(fi, "The face information must be non-null");

  const auto face_type = fi->faceType(std::make_pair(this->number(), this->sys().number()));
  if (face_type == FaceInfo::VarFaceNeighbors::BOTH)
    return Moose::FV::interpolate(*this, face, state);
  else if (auto * bc_pointer = this->getBoundaryCondition(*fi->boundaryIDs().begin()))
  {
    mooseAssert(fi->boundaryIDs().size() == 1, "We should only have one boundary on every face.");

    const auto * original_face_info = bc_pointer->currentFaceInfo();
    const auto original_face_type = bc_pointer->currentFaceType();

    if (fi != original_face_info)
      bc_pointer->setCurrentFaceInfo(fi, face_type);

    const auto boundary_value = bc_pointer->computeBoundaryValue();

    if (fi != original_face_info)
      bc_pointer->setCurrentFaceInfo(original_face_info, original_face_type);

    return boundary_value;
  }
  else
    mooseError("We should never get here!");
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::ValueType
MooseLinearVariableFV<OutputType>::evaluate(const NodeArg & /*node_arg*/,
                                            const StateArg & /*state*/) const
{
  mooseError("Not implemented yet");
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::DotType
MooseLinearVariableFV<OutputType>::evaluateDot(const ElemArg &, const StateArg &) const
{
  mooseError("evaluateDot not implemented for this class of finite volume variables");
}

template <>
ADReal
MooseLinearVariableFV<Real>::evaluateDot(const ElemArg & /*elem_arg*/,
                                         const StateArg & /*state*/) const
{
  mooseError("Not implemented yet");
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::cacheBoundaryBCMap()
{
  _boundary_id_to_bc.clear();
  std::vector<LinearFVBoundaryCondition *> bcs;

  // I believe because query() returns by value but condition returns by reference that binding to a
  // const lvalue reference results in the query() getting destructed and us holding onto a dangling
  // reference. I think that condition returned by value we would be able to bind to a const lvalue
  // reference here. But as it is we'll bind to a regular lvalue
  auto base_query = this->_subproblem.getMooseApp()
                        .theWarehouse()
                        .query()
                        .template condition<AttribSystem>("LinearFVBoundaryCondition")
                        .template condition<AttribThread>(_tid)
                        .template condition<AttribVar>(_var_num)
                        .template condition<AttribSysNum>(this->_sys.number());

  for (const auto bnd_id : this->_mesh.getBoundaryIDs())
  {
    auto base_query_copy = base_query;
    base_query_copy.template condition<AttribBoundaries>(std::set<BoundaryID>({bnd_id}))
        .queryInto(bcs);
    mooseAssert(bcs.size() <= 1, "cannot have multiple BCs on the same boundary");
    if (!bcs.empty())
      _boundary_id_to_bc.emplace(bnd_id, bcs[0]);
  }
}

template <typename OutputType>
LinearFVBoundaryCondition *
MooseLinearVariableFV<OutputType>::getBoundaryCondition(const BoundaryID bd_id) const
{
  const auto iter = _boundary_id_to_bc.find(bd_id);
  if (iter == _boundary_id_to_bc.end())
    return nullptr;
  else
    return iter->second;
}

template class MooseLinearVariableFV<Real>;
