/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INTERNALVOLUME_H
#define INTERNALVOLUME_H

#include "SideIntegralPostprocessor.h"

class InternalVolume;

template <>
InputParameters validParams<InternalVolume>();

/**
 * This class computes the volume of an interior space.
 *
 * This class relies on being handed sidesets that define a closed space.
 *
 * If the sideset defines an interior surface, the volume reported will be
 * positive.  If the sideset defines an exterior surface, the volume
 * reported will be negative.  It is therefore possible to compute the net
 * interior volume by including an interior and an exterior surface
 * in the same sideset.
 */
class InternalVolume : public SideIntegralPostprocessor
{
public:
  InternalVolume(const InputParameters & parameters);

  void initialSetup();

protected:
  virtual Real computeQpIntegral();
  virtual Real getValue();

  const unsigned int _component;
  const Real _scale;
  const Real _addition;
};

#endif // INTERNALVOLUME_H
