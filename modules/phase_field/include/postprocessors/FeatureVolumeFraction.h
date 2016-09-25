/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FEATUREVOLUMEFRACTION_H
#define FEATUREVOLUMEFRACTION_H

#include "GeneralVectorPostprocessor.h"

//Forward Declarations
class FeatureVolumeFraction;

template<>
InputParameters validParams<FeatureVolumeFraction>();

class FeatureVolumeFraction : public GeneralVectorPostprocessor
{
public:
  FeatureVolumeFraction(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  Real calculateAvramiValue();

  VectorPostprocessorValue & _avrami_data;

  const PostprocessorValue & _mesh_volume;
  const VectorPostprocessorValue & _feature_volumes;

  Real _volume_fraction;
  Real _equil_fraction;
};

#endif //FEATUREVOLUMEFRACTION_H
