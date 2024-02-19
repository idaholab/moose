//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorBinnedValuesDivision.h"
#include "MooseMesh.h"
#include "MooseFunctorArguments.h"

#include "libmesh/mesh_base.h"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", FunctorBinnedValuesDivision);

InputParameters
FunctorBinnedValuesDivision::validParams()
{
  InputParameters params = MeshDivision::validParams();
  params.addClassDescription(
      "Divide the mesh along based on uniformly binned values of a functor.");
  params.addRequiredParam<Real>("min_value", "Minimum value of the functor for the binning");
  params.addRequiredParam<Real>("max_value", "Maximum value of the functor for the binning");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_bins", "num_bins>0", "Number of uniform bins in functor values");
  params.addRequiredParam<MooseFunctorName>(
      "functor", "Functor to evaluate to assign points/elements to the bins");
  params.addParam<bool>("assign_out_of_bounds_to_extreme_bins",
                        false,
                        "Whether to map functor values outside of the [min,max] range to the "
                        "lowest and highest bins, or to use the invalid division index");

  return params;
}

FunctorBinnedValuesDivision::FunctorBinnedValuesDivision(const InputParameters & parameters)
  : MeshDivision(parameters),
    NonADFunctorInterface(this),
    _min(getParam<Real>("min_value")),
    _max(getParam<Real>("max_value")),
    _nbins(getParam<unsigned int>("num_bins")),
    _functor(getFunctor<Real>("functor")),
    _oob_is_edge_bins(getParam<bool>("assign_out_of_bounds_to_extreme_bins"))
{
  if (MooseUtils::absoluteFuzzyLessEqual(_max, _min))
    paramError("max_value", "Maximum value should be above minimum value.");
  FunctorBinnedValuesDivision::initialize();
}

void
FunctorBinnedValuesDivision::initialize()
{
  setNumDivisions(_nbins);

  // We could alternatively check every point in the mesh but it seems expensive
  // the functor values can also change so this check would need to be done regularly
  _mesh_fully_indexed = true;
  if (!_oob_is_edge_bins)
    _mesh_fully_indexed = false;
}

unsigned int
FunctorBinnedValuesDivision::getBinIndex(Real value, const Point & pt) const
{
  // Handle out of bounds functor values
  if (!_oob_is_edge_bins)
  {
    if (value < _min)
      return MooseMeshDivision::INVALID_DIVISION_INDEX;
    else if (value > _max)
      return MooseMeshDivision::INVALID_DIVISION_INDEX;
  }
  else
  {
    if (value < _min)
      return 0;
    else if (value > _max)
      return _nbins - 1;
  }

  for (const auto i_bin : make_range(_nbins + 1))
  {
    const auto border_value = _min + i_bin / _nbins * (_max - _min);
    if (MooseUtils::absoluteFuzzyEqual(value, border_value))
      mooseWarning("Functor value " + std::to_string(value) +
                   " is on a bin edge for evaluation near " + Moose::stringify(pt));
    if (value < border_value)
      return (i_bin > 0) ? i_bin - 1 : 0;
  }
  return _nbins;
}

unsigned int
FunctorBinnedValuesDivision::divisionIndex(const Elem & elem) const
{
  Moose::ElemArg elem_arg = {&elem, false};
  Moose::StateArg time_arg = {0, Moose::SolutionIterationType::Time};
  return getBinIndex(_functor(elem_arg, time_arg), elem.vertex_average());
}

unsigned int
FunctorBinnedValuesDivision::divisionIndex(const Point & pt) const
{
  Moose::StateArg time_arg = {0, Moose::SolutionIterationType::Time};

  const auto & pl = _mesh.getMesh().sub_point_locator();
  // There could be more than one elem if we are on the edge between two elements
  std::set<const Elem *> candidates;
  (*pl)(pt, candidates);

  // By convention we will use the element with the lowest element id
  const Elem * elem = nullptr;
  unsigned int min_elem_id = libMesh::invalid_uint;
  for (const auto elem_ptr : candidates)
    if (elem_ptr->id() < min_elem_id)
    {
      elem = elem_ptr;
      min_elem_id = elem_ptr->id();
    }
  if (!elem)
    mooseError("Division index queried for a point outside the local mesh");
  Moose::ElemPointArg elem_pt_arg = {elem, pt, false};

  // Find the element with the lowest id to form an ElemPt argument
  return getBinIndex(_functor(elem_pt_arg, time_arg), pt);
}
