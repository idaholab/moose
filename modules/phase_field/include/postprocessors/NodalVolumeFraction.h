/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NODALVOLUMEFRACTION_H
#define NODALVOLUMEFRACTION_H

#include "FeatureFloodCount.h"

//Forward Declarations
class NodalVolumeFraction;

template<>
InputParameters validParams<NodalVolumeFraction>();

class NodalVolumeFraction : public FeatureFloodCount
{
public:
  NodalVolumeFraction(const InputParameters & parameters);

  virtual void finalize();

  Real getValue();

  void calculateBubbleFraction();
  Real calculateAvramiValue();

protected:
  const PostprocessorValue & _mesh_volume;
  Real _volume_fraction;
  Real _equil_fraction;
};

#endif //NODALVOLUMEFRACTION_H
