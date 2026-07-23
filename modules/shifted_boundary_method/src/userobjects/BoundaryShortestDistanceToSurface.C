//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryShortestDistanceToSurface.h"
#include "SBMUtils.h"
#include "Function.h"

registerMooseObject("ShiftedBoundaryMethodApp", BoundaryShortestDistanceToSurface);

using ElemSide = BoundaryShortestDistanceToSurface::ElemSide;

InputParameters
BoundaryShortestDistanceToSurface::validParams()
{
  InputParameters params = SideUserObject::validParams();

  params.addParam<std::vector<FunctionName>>(
      "surfaces", {}, "Level-set or mesh-based functions that define the boundary distances.");
  params.addParam<std::vector<bool>>(
      "flip_normals", {}, "Whether to flip the direction of the normal vectors.");

  params.addParam<bool>("local_true_normal_correct",
                        false,
                        "If true, the local true normal direction will be corrected to match the "
                        "direction of the local surrogate normal.");

  params.addParam<bool>("suppress_distance_warning",
                        false,
                        "If true, warnings about large distances will be suppressed.");

  params.addParam<bool>("debug_output", false, "If true, debug output will be enabled.");

  params.addClassDescription("Provides surrogateDistance and trueNormal via function or KDTree.");
  return params;
}

BoundaryShortestDistanceToSurface::BoundaryShortestDistanceToSurface(
    const InputParameters & parameters)
  : SideUserObject(parameters),
    _local_true_normal_correct(getParam<bool>("local_true_normal_correct")),
    _surrogate_dot_true_normal_sums_has_reduced(false),
    _suppress_distance_warning(getParam<bool>("suppress_distance_warning")),
    _debug_output(getParam<bool>("debug_output"))
{
  const auto function_names = getParam<std::vector<FunctionName>>("surfaces");
  if (function_names.empty())
    paramError("surfaces", "BoundaryShortestDistanceToSurface requires at least one surface.");
  _distance_functions = SBMUtils::buildDistanceFunctions(function_names, *this);

  const auto num_funcs = _distance_functions.size();

  std::vector<BoundaryName> boundary_names = getParam<std::vector<BoundaryName>>("boundary");

  for (unsigned int j = 0; j < boundary_names.size(); ++j)
  {
    BoundaryID id = _mesh.getBoundaryID(boundary_names[j]);
    _side_id_index[id] = j;
  }

  if (num_funcs != 1 && boundary_names.size() != num_funcs)
    paramError("surfaces", "Number of surfaces must match number of boundaries.");

  std::vector<bool> flip_normals_param = getParam<std::vector<bool>>("flip_normals");
  if (flip_normals_param.empty())
    _flip_normals.resize(boundary_names.size(), false);
  else if (flip_normals_param.size() == 1)
    _flip_normals = std::vector<bool>(boundary_names.size(), flip_normals_param[0]);
  else if (flip_normals_param.size() == boundary_names.size())
    _flip_normals = flip_normals_param;
  else
    paramError("flip_normals",
               "Size of flip_normals must be either 1 or match number of boundaries.");

  // Accumulator for the boundary-wise integral of (surrogate normal . true normal), used in
  // finalize() to canonicalize the sign of the true normal on each boundary.
  _surrogate_dot_true_normal_sums.resize(boundary_names.size(), 0.0);
}

void
BoundaryShortestDistanceToSurface::execute()
{
  const unsigned int n_qp = _qrule->n_points();

  const BoundaryShortestDistanceToSurface::ElemSide elem_side(_current_elem->id(), _current_side);

  // Resize the vectors
  if (_distance_vectors[elem_side].size() != n_qp)
    _distance_vectors[elem_side].resize(n_qp);

  if (_normal_vectors[elem_side].size() != n_qp)
    _normal_vectors[elem_side].resize(n_qp);

  // The sequence of `surfaces` must match the sequence of boundary IDs{}
  // in `BoundaryShortestDistanceToSurface` when multiple functions are provided.
  const auto side_id_it = _side_id_index.find(_current_boundary_id);
  mooseAssert(side_id_it != _side_id_index.end(), "Boundary ID not registered in side map");
  const unsigned int bid_index = side_id_it->second;
  const unsigned int func_idx =
      (_distance_functions.size() == 1) ? 0 : bid_index; // single function or multiple functions

  bool flip_normal = _flip_normals[bid_index];

  for (unsigned int qp = 0; qp < n_qp; qp++)
  {
    const Point & pt = _q_point[qp];
    const auto * func = _distance_functions.at(func_idx);
    _distance_vectors[elem_side][qp] = SBMUtils::distanceVectorFromFunction(func, pt, _t);
    _normal_vectors[elem_side][qp] = SBMUtils::trueNormalFromFunction(func, pt, _t);

    auto & true_normal = _normal_vectors[elem_side][qp];

    if (flip_normal)
      true_normal *= -1.0;
    else if (_local_true_normal_correct)
      true_normal = (true_normal * _normals[qp] < 0) ? -true_normal : true_normal;

    if (_debug_output)
    {
      const auto true_normal_dot_surrogate_normal = true_normal * _normals[qp];
      const auto nt_tangent = _normals[qp] - true_normal_dot_surrogate_normal * true_normal;

      _console << "[" << name() << "] surface=" << func->name() << " point=" << pt
               << " distance=" << _distance_vectors[elem_side][qp] << " true_normal=" << true_normal
               << " tangent=" << nt_tangent << std::endl;
    }

    // Accumulate the integral of (surrogate normal . true normal) over this boundary so that
    // finalize() can flip the true normals of any boundary whose integral is negative, giving a
    // consistent global orientation.
    _surrogate_dot_true_normal_sums_has_reduced = false;
    _surrogate_dot_true_normal_sums[bid_index] += _normals[qp] * true_normal * _JxW[qp];
    _elem_side_to_bid[elem_side] = bid_index;
  }
}

void
BoundaryShortestDistanceToSurface::finalize()
{
  // Canonicalize the global sign of the true normals: flip the normals of any boundary whose
  // integral of (surrogate normal . true normal) is negative so they align with the surrogate.
  if (!_surrogate_dot_true_normal_sums_has_reduced)
  {
    this->comm().sum(_surrogate_dot_true_normal_sums);

    for (auto & [elem_side, normals] : _normal_vectors /*reference because we want to change*/)
    {
      const auto bid_it = _elem_side_to_bid.find(elem_side);
      if (bid_it == _elem_side_to_bid.end())
        mooseError("BoundaryShortestDistanceToSurface::finalize missing boundary index for "
                   "elem_id = ",
                   elem_side.first,
                   ", side = ",
                   elem_side.second,
                   ".");
      const unsigned int bid_index = bid_it->second;
      if (_surrogate_dot_true_normal_sums[bid_index] < 0.0)
      {
        for (auto & normal : normals)
          normal *= -1.0;
      }
    }
    _surrogate_dot_true_normal_sums_has_reduced = true;
  }
}

const RealVectorValue &
BoundaryShortestDistanceToSurface::surrogateDistance(const ElemSide & elem_side,
                                                     unsigned int qp) const
{
  const auto distance_it = _distance_vectors.find(elem_side);
  if (distance_it == _distance_vectors.end())
    mooseError("BoundaryShortestDistanceToSurface::surrogateDistance missing distance data for "
               "elem_id = ",
               elem_side.first,
               ", side = ",
               elem_side.second,
               ".");

  // Warn if the distance is larger than the element size
  const auto dim = _mesh.dimension();
  const auto * elem = _mesh.elemPtr(elem_side.first);
  Real h = std::pow(elem->volume(), 1.0 / dim);

  if (distance_it->second.at(qp).norm() > std::sqrt(dim) * h && !_suppress_distance_warning)
    mooseWarning(
        "Distance exceeds the estimated element scale. "
        "This may indicate an inaccurate distance function or an incorrect surface selection. "
        "Please verify the provided surface and distance function.");

  return distance_it->second.at(qp);
}

const RealVectorValue &
BoundaryShortestDistanceToSurface::trueNormal(const ElemSide & elem_side, unsigned int qp) const
{
  const auto normal_it = _normal_vectors.find(elem_side);
  if (normal_it == _normal_vectors.end())
    mooseError("BoundaryShortestDistanceToSurface::trueNormal missing normal data for elem_id = ",
               elem_side.first,
               ", side = ",
               elem_side.second,
               ".");
  return normal_it->second.at(qp);
}
