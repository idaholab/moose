//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PressureDrop.h"
#include "MathFVUtils.h"
#include "NSFVUtils.h"
#include "NS.h"

#include <cmath>

registerMooseObject("NavierStokesApp", PressureDrop);

InputParameters
PressureDrop::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addClassDescription(
      "Computes the pressure drop between an upstream and a downstream boundary.");

  params.addRequiredParam<MooseFunctorName>("pressure", "The pressure functor");
  params.addParam<MooseFunctorName>("weighting_functor",
                                    "A vector functor to compute a flux to weigh the pressure for "
                                    "the pressure average computations");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "upstream_boundary", "The upstream surface (must also be specified in 'boundary' parameter");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "downstream_boundary",
      "The downstream surface (must also be specified in 'boundary' parameter");

  params.addParam<MooseEnum>("weighting_interp_method",
                             Moose::FV::interpolationMethods(),
                             "The interpolation to use for the weighting functor.");
  return params;
}

PressureDrop::PressureDrop(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _pressure(getFunctor<Real>("pressure")),
    _weighting_functor(isParamValid("weighting_functor")
                           ? &getFunctor<RealVectorValue>("weighting_functor")
                           : nullptr)
{
  // Examine the variable type of pressure to determine the type of integration
  const auto pressure_name = getParam<MooseFunctorName>("pressure");
  if (!_subproblem.hasVariable(pressure_name))
    paramError("pressure", "Pressure must be a variable");
  const auto * const pressure_var = &_subproblem.getVariable(_tid, pressure_name);
  _qp_integration = dynamic_cast<const MooseVariableFE<Real> *>(pressure_var);

  checkFunctorSupportsSideIntegration<Real>("pressure", _qp_integration);
  if (_weighting_functor)
    checkFunctorSupportsSideIntegration<RealVectorValue>("weighting_functor", _qp_integration);

  if (!_qp_integration)
    Moose::FV::setInterpolationMethod(*this, _weight_interp_method, "weighting_interp_method");

  else if (parameters.isParamSetByUser("weighting_interp_method"))
    paramError("weighting_interp_method", "Face interpolation only specified for finite volume");

  // Only keep the ids
  auto upstream_bdies = getParam<std::vector<BoundaryName>>("upstream_boundary");
  auto downstream_bdies = getParam<std::vector<BoundaryName>>("downstream_boundary");
  _upstream_boundaries.resize(upstream_bdies.size());
  _downstream_boundaries.resize(downstream_bdies.size());
  for (auto i : make_range(upstream_bdies.size()))
    _upstream_boundaries[i] = _mesh.getBoundaryID(upstream_bdies[i]);
  for (auto i : make_range(downstream_bdies.size()))
    _downstream_boundaries[i] = _mesh.getBoundaryID(downstream_bdies[i]);

  // Check that boundary restriction makes sense
  for (auto bdy : _upstream_boundaries)
    if (!hasBoundary(bdy))
      paramError("boundary",
                 "Upstream boundary '" + _mesh.getBoundaryName(bdy) +
                     "' is not included in boundary restriction");
  for (auto bdy : _downstream_boundaries)
    if (!hasBoundary(bdy))
      paramError("boundary",
                 "Downstream boundary '" + _mesh.getBoundaryName(bdy) +
                     "' is not included in boundary restriction");
  for (auto bdy : boundaryIDs())
    if (std::find(_upstream_boundaries.begin(), _upstream_boundaries.end(), bdy) ==
            _upstream_boundaries.end() &&
        std::find(_downstream_boundaries.begin(), _downstream_boundaries.end(), bdy) ==
            _downstream_boundaries.end())
      paramError("boundary",
                 "Boundary restriction on boundary '" + _mesh.getBoundaryName(bdy) +
                     "' is not part of upstream or downstream boundaries");
  for (auto bdy : _upstream_boundaries)
    if (std::find(_downstream_boundaries.begin(), _downstream_boundaries.end(), bdy) !=
        _downstream_boundaries.end())
      paramError("upstream_boundary",
                 "Upstream boundary '" + _mesh.getBoundaryName(bdy) +
                     "' is also a downstream boundary");
}

void
PressureDrop::initialize()
{
  _weighted_pressure_upstream = 0;
  _weighted_pressure_downstream = 0;
  _weight_upstream = 0;
  _weight_downstream = 0;

  // Build the face infos in all cases, needed to detect upstream/downstream status
  if (_mesh.isFiniteVolumeInfoDirty())
    _mesh.setupFiniteVolumeMeshData();
}

void
PressureDrop::meshChanged()
{
  initialize();
}

void
PressureDrop::execute()
{
  // Determine if upstream or downstream boundary
  bool upstream = false;
  bool status_known = false;
  getFaceInfos();

  for (auto & fi : _face_infos)
  {
    for (const auto bdy : fi->boundaryIDs())
    {
      if (std::find(_upstream_boundaries.begin(), _upstream_boundaries.end(), bdy) !=
          _upstream_boundaries.end())
      {
        upstream = true;
        status_known = true;
#ifdef NDEBUG
        break;
#else
        if (std::find(_downstream_boundaries.begin(), _downstream_boundaries.end(), bdy) !=
            _downstream_boundaries.end())
          mooseError("Boundary '", _mesh.getBoundaryName(bdy), "' is both upstream and downstream");
#endif
      }
#ifndef NDEBUG
      if (upstream &&
          std::find(_downstream_boundaries.begin(), _downstream_boundaries.end(), bdy) !=
              _downstream_boundaries.end())
        mooseError("Face info is part of both upstream and downstream boundaries");
#endif
      if (!upstream &&
          std::find(_downstream_boundaries.begin(), _downstream_boundaries.end(), bdy) !=
              _downstream_boundaries.end())
        status_known = true;

        // in debug mode we will check all boundaries the face info is a part of
        // to make sure they are consistently upstream or downstream
#ifdef NDEBUG
      if (status_known)
        break;
#endif
    }
    // we ll assume all face infos for a given element and side have the same upstream status
    // it seems very unlikely that upstream and downstream boundaries would be touching
    // and the delimitation would be within a refined element's face
    if (status_known)
      break;
  }

  if (upstream)
  {
    // Integration loops are different for FE and FV at this point
    if (_qp_integration)
    {
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        _weighted_pressure_upstream +=
            _JxW[_qp] * _coord[_qp] * computeQpWeightedPressureIntegral();
        _weight_upstream += _JxW[_qp] * _coord[_qp] * computeQpWeightIntegral();
      }
    }
    else
    {
      for (auto & fi : _face_infos)
      {
        _weighted_pressure_upstream +=
            fi->faceArea() * fi->faceCoord() * computeFaceInfoWeightedPressureIntegral(fi);
        _weight_upstream += fi->faceArea() * fi->faceCoord() * computeFaceInfoWeightIntegral(fi);
      }
    }
  }
  // Downstream contributions
  else
  {
    if (_qp_integration)
    {
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        _weighted_pressure_downstream +=
            _JxW[_qp] * _coord[_qp] * computeQpWeightedPressureIntegral();
        _weight_downstream += _JxW[_qp] * _coord[_qp] * computeQpWeightIntegral();
      }
    }
    else
    {
      for (auto & fi : _face_infos)
      {
        _weighted_pressure_downstream +=
            fi->faceArea() * fi->faceCoord() * computeFaceInfoWeightedPressureIntegral(fi);
        _weight_downstream += fi->faceArea() * fi->faceCoord() * computeFaceInfoWeightIntegral(fi);
      }
    }
  }
}

Real
PressureDrop::computeFaceInfoWeightedPressureIntegral(const FaceInfo * const fi) const
{
  mooseAssert(fi, "We should have a face info in " + name());

  const bool correct_skewness =
      _weight_interp_method == Moose::FV::InterpMethod::SkewCorrectedAverage;
  const Moose::ElemQpArg elem_arg = {_current_elem, _qp, _qrule, _q_point[_qp]};
  const auto state = determineState();
  mooseAssert(_qp == 0, "Only one quadrature point");
  mooseAssert(_pressure.hasFaceSide(*fi, true) || _pressure.hasFaceSide(*fi, true),
              "Pressure must be defined at least on one side of the face!");
  const auto * elem = _pressure.hasFaceSide(*fi, true) ? fi->elemPtr() : fi->neighborPtr();

  if (_weighting_functor)
  {
    mooseAssert(_pressure.hasFaceSide(*fi, true) == _weighting_functor->hasFaceSide(*fi, true),
                "Pressure and weighting functor have to be defined on the same side of the face!");
    const auto ssf = Moose::FaceArg(
        {fi,
         Moose::FV::limiterType(_weight_interp_method),
         MetaPhysicL::raw_value((*_weighting_functor)(elem_arg, state)) * fi->normal() > 0,
         correct_skewness,
         elem,
         nullptr});
    const auto face_weighting = MetaPhysicL::raw_value((*_weighting_functor)(ssf, state));
    return fi->normal() * face_weighting * _pressure(ssf, state);
  }
  else
  {
    const auto ssf = Moose::FaceArg(
        {fi, Moose::FV::limiterType(_weight_interp_method), true, correct_skewness, elem, nullptr});
    return _pressure(ssf, state);
  }
}

Real
PressureDrop::computeFaceInfoWeightIntegral(const FaceInfo * fi) const
{
  mooseAssert(fi, "We should have a face info in " + name());
  const Moose::ElemQpArg elem_arg = {_current_elem, _qp, _qrule, _q_point[_qp]};
  mooseAssert(_qp == 0, "Only one quadrature point");
  const auto state = determineState();

  const bool correct_skewness =
      _weight_interp_method == Moose::FV::InterpMethod::SkewCorrectedAverage;

  if (_weighting_functor)
  {
    const auto ssf = Moose::FaceArg(
        {fi,
         Moose::FV::limiterType(_weight_interp_method),
         MetaPhysicL::raw_value((*_weighting_functor)(elem_arg, state)) * fi->normal() > 0,
         correct_skewness,
         _weighting_functor->hasFaceSide(*fi, true) ? fi->elemPtr() : fi->neighborPtr(),
         nullptr});
    return fi->normal() * MetaPhysicL::raw_value((*_weighting_functor)(ssf, state));
  }
  else
    return 1;
}

Real
PressureDrop::computeQpWeightedPressureIntegral() const
{
  const Moose::ElemSideQpArg elem_side_arg = {
      _current_elem, _current_side, _qp, _qrule, _q_point[_qp]};
  const auto state = determineState();
  if (_weighting_functor)
    return (*_weighting_functor)(elem_side_arg, state) * _normals[_qp] *
           _pressure(elem_side_arg, state);
  else
    return _pressure(elem_side_arg, state);
}

Real
PressureDrop::computeQpWeightIntegral() const
{
  const Moose::ElemSideQpArg elem_side_arg = {
      _current_elem, _current_side, _qp, _qrule, _q_point[_qp]};
  const auto state = determineState();
  if (_weighting_functor)
    return (*_weighting_functor)(elem_side_arg, state) * _normals[_qp];
  else
    return 1;
}

void
PressureDrop::threadJoin(const UserObject & y)
{
  const auto & pps = static_cast<const PressureDrop &>(y);
  _weighted_pressure_upstream += pps._weighted_pressure_upstream;
  _weighted_pressure_downstream += pps._weighted_pressure_downstream;
  _weight_upstream += pps._weight_upstream;
  _weight_downstream += pps._weight_downstream;
}

void
PressureDrop::finalize()
{
  gatherSum(_weighted_pressure_upstream);
  gatherSum(_weighted_pressure_downstream);
  gatherSum(_weight_upstream);
  gatherSum(_weight_downstream);
}

Real
PressureDrop::getValue() const
{
  if (MooseUtils::absoluteFuzzyEqual(_weight_upstream, 0) ||
      MooseUtils::absoluteFuzzyEqual(_weight_downstream, 0))
  {
    mooseWarning("Weight integral is 0 (downstream or upstream), either :\n"
                 "- pressure drop value is queried before being computed\n"
                 "- the weight flux integral is simply 0, the weighting is not appropriate");
    return 0;
  }
  return _weighted_pressure_upstream / _weight_upstream -
         _weighted_pressure_downstream / _weight_downstream;
}
