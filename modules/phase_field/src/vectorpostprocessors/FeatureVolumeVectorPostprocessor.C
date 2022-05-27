//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FeatureVolumeVectorPostprocessor.h"

// MOOSE includes
#include "Assembly.h"
#include "FeatureFloodCount.h"
#include "GrainTrackerInterface.h"
#include "MooseMesh.h"
#include "MooseVariable.h"

#include "libmesh/quadrature.h"

registerMooseObject("PhaseFieldApp", FeatureVolumeVectorPostprocessor);

InputParameters
FeatureVolumeVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params += BoundaryRestrictable::validParams();

  params.addRequiredParam<UserObjectName>("flood_counter",
                                          "The FeatureFloodCount UserObject to get values from.");
  params.addParam<bool>("single_feature_per_element",
                        false,
                        "Set this Boolean if you wish to use an element based volume where"
                        " the dominant order parameter determines the feature that accumulates the "
                        "entire element volume");
  params.addParam<bool>("output_centroids", false, "Set to true to output the feature centroids");
  params.addClassDescription("This object is designed to pull information from the data structures "
                             "of a \"FeatureFloodCount\" or derived object (e.g. individual "
                             "feature volumes)");

  params.suppressParameter<bool>("contains_complete_history");

  return params;
}

FeatureVolumeVectorPostprocessor::FeatureVolumeVectorPostprocessor(
    const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    MooseVariableDependencyInterface(this),
    BoundaryRestrictable(this, false),
    _single_feature_per_elem(getParam<bool>("single_feature_per_element")),
    _output_centroids(getParam<bool>("output_centroids")),
    _feature_counter(getUserObject<FeatureFloodCount>("flood_counter")),
    _var_num(declareVector("var_num")),
    _feature_volumes(declareVector("feature_volumes")),
    _intersects_bounds(declareVector("intersects_bounds")),
    _intersects_specified_bounds(declareVector("intersects_specified_bounds")),
    _percolated(declareVector("percolated")),
    _vars(_feature_counter.getFECoupledVars()),
    _mesh(_subproblem.mesh()),
    _assembly(_subproblem.assembly(_tid)),
    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation()),
    _qrule_face(_assembly.qRuleFace()),
    _JxW_face(_assembly.JxWFace())
{
  addMooseVariableDependency(_vars);

  _is_boundary_restricted = boundaryRestricted();

  _coupled_sln.reserve(_vars.size());
  for (auto & var : _feature_counter.getCoupledVars())
    _coupled_sln.push_back(&var->sln());

  const std::array<std::string, 3> suffix = {{"x", "y", "z"}};
  if (_output_centroids)
    for (unsigned int i = 0; i < 3; ++i)
      _centroid[i] = &declareVector("centroid_" + suffix[i]);
}

void
FeatureVolumeVectorPostprocessor::initialize()
{
}

void
FeatureVolumeVectorPostprocessor::execute()
{
  const auto num_features = _feature_counter.getTotalFeatureCount();

  // Reset the variable index and intersect bounds vectors
  _var_num.assign(num_features, -1);                     // Invalid
  _intersects_bounds.assign(num_features, -1);           // Invalid
  _intersects_specified_bounds.assign(num_features, -1); // Invalid
  _percolated.assign(num_features, -1);                  // Invalid
  for (MooseIndex(num_features) feature_num = 0; feature_num < num_features; ++feature_num)
  {
    auto var_num = _feature_counter.getFeatureVar(feature_num);
    if (var_num != FeatureFloodCount::invalid_id)
      _var_num[feature_num] = var_num;

    _intersects_bounds[feature_num] =
        static_cast<unsigned int>(_feature_counter.doesFeatureIntersectBoundary(feature_num));

    _intersects_specified_bounds[feature_num] = static_cast<unsigned int>(
        _feature_counter.doesFeatureIntersectSpecifiedBoundary(feature_num));

    _percolated[feature_num] =
        static_cast<unsigned int>(_feature_counter.isFeaturePercolated(feature_num));
  }

  if (_output_centroids)
  {
    for (std::size_t i = 0; i < 3; ++i)
      _centroid[i]->resize(num_features);
    for (std::size_t feature_num = 0; feature_num < num_features; ++feature_num)
    {
      auto p = _feature_counter.featureCentroid(feature_num);
      for (std::size_t i = 0; i < 3; ++i)
        (*_centroid[i])[feature_num] = p(i);
    }
  }

  // Reset the volume vector
  _feature_volumes.assign(num_features, 0);

  // Calculate coverage of a boundary if one has been supplied in the input file
  if (_is_boundary_restricted)
  {
    const std::set<BoundaryID> supplied_bnd_ids = BoundaryRestrictable::boundaryIDs();
    for (auto elem_it = _mesh.bndElemsBegin(), elem_end = _mesh.bndElemsEnd(); elem_it != elem_end;
         ++elem_it)

      // loop over only boundaries supplied by user in boundary param
      for (auto & supplied_bnd_id : supplied_bnd_ids)
        if (((*elem_it)->_bnd_id) == supplied_bnd_id)
        {
          const auto & elem = (*elem_it)->_elem;
          auto rank = processor_id();

          if (elem->processor_id() == rank)
          {
            _fe_problem.setCurrentSubdomainID(elem, 0);
            _fe_problem.prepare(elem, 0);
            _fe_problem.reinitElem(elem, 0);
            _fe_problem.reinitElemFace(elem, (*elem_it)->_side, (*elem_it)->_bnd_id, 0);

            const auto & var_to_features = _feature_counter.getVarToFeatureVector(elem->id());

            accumulateBoundaryFaces(elem, var_to_features, num_features, (*elem_it)->_side);
          }
        }
  }
  else // If no boundary is supplied, calculate volumes of features as normal
    for (const auto & elem : _mesh.getMesh().active_local_element_ptr_range())
    {
      _fe_problem.setCurrentSubdomainID(elem, 0);
      _fe_problem.prepare(elem, 0);
      _fe_problem.reinitElem(elem, 0);

      /**
       * Here we retrieve the var to features vector on the current element.
       * We'll use that information to figure out which variables are non-zero
       * (from a threshold perspective) then we can sum those values into
       * appropriate grain index locations.
       */
      const auto & var_to_features = _feature_counter.getVarToFeatureVector(elem->id());

      accumulateVolumes(elem, var_to_features, num_features);
    }
}

void
FeatureVolumeVectorPostprocessor::finalize()
{
  // Do the parallel sum
  _communicator.sum(_feature_volumes);
}

Real
FeatureVolumeVectorPostprocessor::getFeatureVolume(unsigned int feature_id) const
{
  mooseAssert(feature_id < _feature_volumes.size(), "feature_id is out of range");
  return _feature_volumes[feature_id];
}

void
FeatureVolumeVectorPostprocessor::accumulateVolumes(
    const Elem * elem,
    const std::vector<unsigned int> & var_to_features,
    std::size_t libmesh_dbg_var(num_features))
{
  unsigned int dominant_feature_id = FeatureFloodCount::invalid_id;
  Real max_var_value = std::numeric_limits<Real>::lowest();

  for (MooseIndex(var_to_features) var_index = 0; var_index < var_to_features.size(); ++var_index)
  {
    // Only sample "active" variables
    if (var_to_features[var_index] != FeatureFloodCount::invalid_id)
    {
      auto feature_id = var_to_features[var_index];
      mooseAssert(feature_id < num_features, "Feature ID out of range");
      auto integral_value = computeIntegral(var_index);

      // Compute volumes in a simplistic but domain conservative fashion
      if (_single_feature_per_elem)
      {
        if (integral_value > max_var_value)
        {
          // Update the current dominant feature and associated value
          max_var_value = integral_value;
          dominant_feature_id = feature_id;
        }
      }
      // Solution based volume calculation (integral value)
      else
        _feature_volumes[feature_id] += integral_value;
    }
  }

  // Accumulate the entire element volume into the dominant feature. Do not use the integral value
  if (_single_feature_per_elem && dominant_feature_id != FeatureFloodCount::invalid_id)
    _feature_volumes[dominant_feature_id] += _assembly.elementVolume(elem);
}

Real
FeatureVolumeVectorPostprocessor::computeIntegral(std::size_t var_index) const
{
  Real sum = 0;

  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    sum += _JxW[qp] * _coord[qp] * (*_coupled_sln[var_index])[qp];

  return sum;
}

void
FeatureVolumeVectorPostprocessor::accumulateBoundaryFaces(
    const Elem * elem,
    const std::vector<unsigned int> & var_to_features,
    std::size_t libmesh_dbg_var(num_features),
    unsigned int side)
{
  unsigned int dominant_feature_id = FeatureFloodCount::invalid_id;
  Real max_var_value = std::numeric_limits<Real>::lowest();

  for (MooseIndex(var_to_features) var_index = 0; var_index < var_to_features.size(); ++var_index)
  {
    // Only sample "active" variables
    if (var_to_features[var_index] != FeatureFloodCount::invalid_id)
    {
      auto feature_id = var_to_features[var_index];
      mooseAssert(feature_id < num_features, "Feature ID out of range");
      auto integral_value = computeFaceIntegral(var_index);

      if (_single_feature_per_elem)
      {
        if (integral_value > max_var_value)
        {
          // Update the current dominant feature and associated value
          max_var_value = integral_value;
          dominant_feature_id = feature_id;
        }
      }
      // Solution based boundary area/length calculation (integral value)
      else
        _feature_volumes[feature_id] += integral_value;
    }
  }

  // Accumulate the boundary area/length into the dominant feature. Do not use the integral value
  if (_single_feature_per_elem && dominant_feature_id != FeatureFloodCount::invalid_id)
  {
    std::unique_ptr<const Elem> side_elem = elem->build_side_ptr(side);
    _feature_volumes[dominant_feature_id] += _assembly.elementVolume(side_elem.get());
  }
}

Real
FeatureVolumeVectorPostprocessor::computeFaceIntegral(std::size_t var_index) const
{
  Real sum = 0;
  for (unsigned int qp = 0; qp < _qrule_face->n_points(); ++qp)
    sum += _JxW_face[qp] * _coord[qp] * (*_coupled_sln[var_index])[qp];

  return sum;
}
