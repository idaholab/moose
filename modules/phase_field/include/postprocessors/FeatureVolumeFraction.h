/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FEATUREVOLUMEFRACTION_H
#define FEATUREVOLUMEFRACTION_H

#include "FeatureFloodCount.h"

//Forward Declarations
class FeatureVolumeFraction;

template<>
InputParameters validParams<FeatureVolumeFraction>();

class FeatureVolumeFraction : public FeatureFloodCount
{
public:
  FeatureVolumeFraction(const InputParameters & parameters);

  virtual void finalize();

  Real getValue();

  void calculateBubbleFraction();
  Real calculateAvramiValue();

protected:
  const PostprocessorValue & _mesh_volume;
  Real _volume_fraction;
  Real _equil_fraction;
};

#endif //FEATUREVOLUMEFRACTION_H
