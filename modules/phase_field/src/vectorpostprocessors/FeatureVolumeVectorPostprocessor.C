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
  return params;
}

FeatureVolumeVectorPostprocessor::FeatureVolumeVectorPostprocessor(const InputParameters & parameters) :
    GeneralVectorPostprocessor(parameters),
    MooseVariableDependencyInterface(),
    _feature_counter(getUserObject<FeatureFloodCount>("flood_counter")),
    _feature_volumes(declareVector("feature_volumes")),
    _feature_ids(declareVector("feature_ids")),
    _var_volume_intersect_bounds(declareVector("op_volume_intersect_bounds")),
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

  // Reset all of the vectors
  _feature_volumes.assign(num_features, 0);
  _feature_ids.assign(num_features, -1);
  _var_volume_intersect_bounds.assign(_vars.size(), 0);

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

    for (auto var_index = beginIndex(var_to_features); var_index < var_to_features.size(); ++var_index)
    {
      // Only sample "active" variables
      if (var_to_features[var_index] != FeatureFloodCount::invalid_id)
      {
        auto feature_id = var_to_features[var_index];
        mooseAssert(feature_id < num_features, "Feature ID out of range");

        // Add in the integral value on the current variable to the current feature's slot
        Real integral_value = computeIntegral(var_index);
        _feature_volumes[feature_id] += integral_value;

        if (_feature_counter.doesFeatureIntersectBoundary(feature_id))
          _var_volume_intersect_bounds[var_index] += integral_value;
      }
    }
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

Real
FeatureVolumeVectorPostprocessor::computeIntegral(std::size_t var_index) const
{
  Real sum = 0;

  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    sum += _JxW[qp] * _coord[qp] * (*_coupled_sln[var_index])[qp];

  return sum;
}
