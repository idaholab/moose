/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
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
