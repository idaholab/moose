/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FEATUREVOLUMEFRACTION_H
#define FEATUREVOLUMEFRACTION_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class FeatureVolumeFraction;

template <>
InputParameters validParams<FeatureVolumeFraction>();

class FeatureVolumeFraction : public GeneralPostprocessor
{
public:
  FeatureVolumeFraction(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;

protected:
  Real calculateAvramiValue();

  enum class ValueType
  {
    VOLUME_FRACTION,
    AVRAMI,
  };

  const ValueType _value_type;
  const PostprocessorValue & _mesh_volume;
  const VectorPostprocessorValue & _feature_volumes;

  Real _volume_fraction;
  Real _equil_fraction;
  Real _avrami_value;
};

#endif // FEATUREVOLUMEFRACTION_H
