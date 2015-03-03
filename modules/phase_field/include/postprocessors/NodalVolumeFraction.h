/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef NODALVOLUMEFRACTION_H
#define NODALVOLUMEFRACTION_H

#include "NodalFloodCount.h"

//Forward Declarations
class NodalVolumeFraction;

template<>
InputParameters validParams<NodalVolumeFraction>();

class NodalVolumeFraction : public NodalFloodCount
{
public:
  NodalVolumeFraction(const std::string & name, InputParameters parameters);
  ~NodalVolumeFraction();

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
