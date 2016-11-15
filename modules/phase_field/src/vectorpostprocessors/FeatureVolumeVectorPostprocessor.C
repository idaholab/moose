/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "FeatureVolumeVectorPostprocessor.h"
#include "FeatureFloodCount.h"
#include "GrainTrackerInterface.h"
#include "MooseMesh.h"
#include "Assembly.h"

#include "libmesh/quadrature.h"

template<>
InputParameters validParams<FeatureVolumeVectorPostprocessor>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();

  params.addRequiredParam<UserObjectName>("flood_counter", "The FeatureFloodCount UserObject to get values from.");
  params.addParam<bool>("single_feature_per_element", false, "Set this Boolean if you wish to use an element based volume where"
                        " the dominant order parameter determines the feature that accumulates the entire element volume");
  return params;
}

FeatureVolumeVectorPostprocessor::FeatureVolumeVectorPostprocessor(const InputParameters & parameters) :
    GeneralVectorPostprocessor(parameters),
    MooseVariableDependencyInterface(this),
    _single_feature_per_elem(getParam<bool>("single_feature_per_element")),
    _feature_counter(getUserObject<FeatureFloodCount>("flood_counter")),
    _var_num(declareVector("var_num")),
    _feature_volumes(declareVector("feature_volumes")),
    _intersects_bounds(declareVector("intersects_bounds")),
    _vars(_feature_counter.getCoupledVars()),
    _mesh(_subproblem.mesh()),
    _assembly(_subproblem.assembly(_tid)),
    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation())
{
  addMooseVariableDependency(_vars);

  _coupled_sln.reserve(_vars.size());
  for (auto & var : _vars)
    _coupled_sln.push_back(&var->sln());
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
  _var_num.assign(num_features, -1); // Invalid
  _intersects_bounds.assign(num_features, -1); // Invalid
  for (auto feature_num = beginIndex(_var_num); feature_num < num_features; ++feature_num)
  {
    auto var_num = _feature_counter.getFeatureVar(feature_num);
    if (var_num != FeatureFloodCount::invalid_id)
      _var_num[feature_num] = var_num;

    _intersects_bounds[feature_num] = static_cast<unsigned int>(_feature_counter.doesFeatureIntersectBoundary(feature_num));
  }

  // Reset the volume vector
  _feature_volumes.assign(num_features, 0);
  const auto end = _mesh.getMesh().active_local_elements_end();
  for (auto el = _mesh.getMesh().active_local_elements_begin(); el != end; ++el)
  {
    const Elem * elem = *el;
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
FeatureVolumeVectorPostprocessor::accumulateVolumes(const Elem * elem, const std::vector<unsigned int> & var_to_features, std::size_t libmesh_dbg_var(num_features))
{
  unsigned int dominant_feature_id = FeatureFloodCount::invalid_id;
  Real max_var_value = std::numeric_limits<Real>::lowest();

  for (auto var_index = beginIndex(var_to_features); var_index < var_to_features.size(); ++var_index)
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
    _feature_volumes[dominant_feature_id] += elem->volume();
}

Real
FeatureVolumeVectorPostprocessor::computeIntegral(std::size_t var_index) const
{
  Real sum = 0;

  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    sum += _JxW[qp] * _coord[qp] * (*_coupled_sln[var_index])[qp];

  return sum;
}
