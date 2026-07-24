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
#include "SBMSurfaceMeshBuilder.h"
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

  params.addParam<Point>(
      "at_point",
      "Optional fixed query point. If supplied, the accessor is evaluated at this "
      "point instead of the current element's centroid (useful for hitting "
      "zero-distance branches at a surface-mesh node).");

  params.addParam<UserObjectName>(
      "builder",
      "Optional SBMSurfaceMeshBuilder; when supplied, initialSetup queries "
      "getCentroids() so coverage tests can touch that otherwise-unused getter.");

  params.addClassDescription("Test-only AuxKernel that exposes ShortestDistanceToSurface accessors "
                             "(trueNormal, *ByIndex, *ByFunc) for coverage testing.");

  return params;
}

ShortestDistanceToSurfaceTestAux::ShortestDistanceToSurfaceTestAux(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _distance_to_surface(getUserObject<ShortestDistanceToSurface>("distance_to_surface")),
    _builder(isParamValid("builder") ? &getUserObject<SBMSurfaceMeshBuilder>("builder") : nullptr),
    _method(getParam<MooseEnum>("method")),
    _component(getParam<MooseEnum>("component")),
    _index(getParam<unsigned int>("index")),
    _function(isParamValid("function") ? &getFunction("function") : nullptr),
    _use_at_point(isParamValid("at_point")),
    _at_point(_use_at_point ? getParam<Point>("at_point") : Point())
{
  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");

  const bool needs_function = _method == "distance_by_func" || _method == "true_normal_by_func";
  if (needs_function && !_function)
    paramError("function", "A 'function' parameter is required for the *_by_func methods.");
}

void
ShortestDistanceToSurfaceTestAux::initialSetup()
{
  if (_builder && _builder->getCentroids().empty())
    mooseError("ShortestDistanceToSurfaceTestAux: builder '",
               _builder->name(),
               "' returned no centroids.");
}

Real
ShortestDistanceToSurfaceTestAux::computeValue()
{
  const Point pt = _use_at_point ? _at_point : _current_elem->vertex_average();

  RealVectorValue v;
  if (_method == "true_normal")
    v = _distance_to_surface.trueNormal(pt);
  else if (_method == "distance_by_index")
    v = _distance_to_surface.distanceVectorByIndex(_index, pt);
  else if (_method == "true_normal_by_index")
    v = _distance_to_surface.trueNormalByIndex(_index, pt);
  else if (_method == "distance_by_func")
    v = _distance_to_surface.distanceVectorByFunc(pt, _t, _function);
  else
    v = _distance_to_surface.trueNormalByFunc(pt, _t, _function);

  if (_component == "x")
    return v(0);
  if (_component == "y")
    return v(1);
  if (_component == "z")
    return v(2);
  return v.norm();
}
