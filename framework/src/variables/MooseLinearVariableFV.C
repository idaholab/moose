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
    _prev_elem(nullptr)
{
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
MooseLinearVariableFV<OutputType>::getElemValue(const Elem * const /*elem*/,
                                                const StateArg & /*state*/) const
{
  mooseError("Not implemented yet");
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
MooseLinearVariableFV<OutputType>::adGradSln(const Elem * const /*elem*/,
                                             const StateArg & /*state*/,
                                             const bool /*correct_skewness*/) const
{
  mooseError("Not implemented yet");
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
MooseLinearVariableFV<OutputType>::adGradSln(const FaceInfo & /*fi*/,
                                             const StateArg & /*state*/,
                                             const bool /*correct_skewness*/) const
{
  mooseError("Not implemented yet");
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::initialSetup()
{
  MooseVariableField<OutputType>::initialSetup();
  cacheBoundaryBCMap();
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::ValueType
MooseLinearVariableFV<OutputType>::evaluate(const FaceArg & /*face*/,
                                            const StateArg & /*state*/) const
{
  mooseError("Not implemented yet");
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
MooseLinearVariableFV<OutputType>::getBoundaryCondition(const BoundaryID bd_id)
{
  const auto iter = _boundary_id_to_bc.find(bd_id);
  if (iter == _boundary_id_to_bc.end())
    return nullptr;
  else
    return iter->second;
}

template class MooseLinearVariableFV<Real>;
