/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "FeatureVolumeFraction.h"
#include <cmath>

template<>
InputParameters validParams<FeatureVolumeFraction>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();
  params.addRequiredParam<PostprocessorName>("mesh_volume", "Postprocessor from which to get mesh volume");
  params.addRequiredParam<VectorPostprocessorName>("grain_volumes", "The feature volume VectorPostprocessorValue.");
  params.addParam<Real>("equil_fraction", -1.0, "Equilibrium volume fraction of 2nd phase for Avrami analysis");
  return params;
}

FeatureVolumeFraction::FeatureVolumeFraction(const InputParameters & parameters) :
    GeneralVectorPostprocessor(parameters),
    _avrami_data(declareVector("avrami_data")),
    _mesh_volume(getPostprocessorValue("mesh_volume")),
    _feature_volumes(getVectorPostprocessorValue("feature_volumes", "feature_volumes")),
    _equil_fraction(getParam<Real>("equil_fraction"))
{
}

void
FeatureVolumeFraction::initialize()
{
}

void
FeatureVolumeFraction::execute()
{
  Real volume = 0.0;

  //sum the values in the vector to get total volume
  for (auto it = _feature_volumes.begin(); it != _feature_volumes.end(); ++it)
    volume += *it;

  _volume_fraction = volume / _mesh_volume;

  _avrami_data = { static_cast<Real>(_fe_problem.timeStep()), _fe_problem.time(), std::log(_fe_problem.time()), calculateAvramiValue() };
}

void
FeatureVolumeFraction::finalize()
{
}

Real
FeatureVolumeFraction::calculateAvramiValue()
{
  return std::log(std::log(1.0 / (1.0 - (_volume_fraction/_equil_fraction))));
}
