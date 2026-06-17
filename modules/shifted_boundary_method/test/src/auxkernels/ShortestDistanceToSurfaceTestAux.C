//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShortestDistanceToSurfaceTestAux.h"
#include "ShortestDistanceToSurface.h"
#include "Function.h"

registerMooseObject("ShiftedBoundaryMethodTestApp", ShortestDistanceToSurfaceTestAux);

InputParameters
ShortestDistanceToSurfaceTestAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addRequiredParam<UserObjectName>("distance_to_surface",
                                          "ShortestDistanceToSurface user object to query.");

  MooseEnum methods("true_normal distance_by_index true_normal_by_index "
                    "distance_by_func true_normal_by_func");
  params.addRequiredParam<MooseEnum>(
      "method", methods, "Which ShortestDistanceToSurface accessor to call.");

  MooseEnum components("x y z norm", "norm");
  params.addParam<MooseEnum>(
      "component", components, "Vector component returned by computeValue (default: norm).");

  params.addParam<unsigned int>("index", 0, "Surface index used for the *ByIndex variants.");

  params.addParam<FunctionName>("function", "Function used for the *ByFunc variants.");

  params.addClassDescription("Test-only AuxKernel that exposes ShortestDistanceToSurface accessors "
                             "(trueNormal, *ByIndex, *ByFunc) for coverage testing.");

  return params;
}

ShortestDistanceToSurfaceTestAux::ShortestDistanceToSurfaceTestAux(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _distance_to_surface(getUserObject<ShortestDistanceToSurface>("distance_to_surface")),
    _method(static_cast<Method>(int(getParam<MooseEnum>("method")))),
    _component(static_cast<Component>(int(getParam<MooseEnum>("component")))),
    _index(getParam<unsigned int>("index")),
    _function(isParamValid("function") ? &getFunction("function") : nullptr)
{
  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");

  const bool needs_function =
      _method == Method::DISTANCE_BY_FUNC || _method == Method::TRUE_NORMAL_BY_FUNC;
  if (needs_function && !_function)
    paramError("function", "A 'function' parameter is required for the *_by_func methods.");
}

Real
ShortestDistanceToSurfaceTestAux::computeValue()
{
  const Point & pt = _current_elem->vertex_average();

  RealVectorValue v;
  switch (_method)
  {
    case Method::TRUE_NORMAL:
      v = _distance_to_surface.trueNormal(pt);
      break;
    case Method::DISTANCE_BY_INDEX:
      v = _distance_to_surface.distanceVectorByIndex(_index, pt);
      break;
    case Method::TRUE_NORMAL_BY_INDEX:
      v = _distance_to_surface.trueNormalByIndex(_index, pt);
      break;
    case Method::DISTANCE_BY_FUNC:
      v = _distance_to_surface.distanceVectorByFunc(pt, _t, _function);
      break;
    case Method::TRUE_NORMAL_BY_FUNC:
      v = _distance_to_surface.trueNormalByFunc(pt, _t, _function);
      break;
  }

  switch (_component)
  {
    case Component::X:
      return v(0);
    case Component::Y:
      return v(1);
    case Component::Z:
      return v(2);
    case Component::NORM:
    default:
      return v.norm();
  }
}
