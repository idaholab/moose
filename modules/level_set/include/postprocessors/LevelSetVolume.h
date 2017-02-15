/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LEVELSETVOLUME_H
#define LEVELSETVOLUME_H

// MOOSE includes
#include "ElementVariablePostprocessor.h"

// Forward declerations
class LevelSetVolume;

template<>
InputParameters validParams<LevelSetVolume>();

/**
 * Postprocessor to compute the area/volume inside and outside of a level set contour.
 */
class LevelSetVolume : public ElementVariablePostprocessor
{
public:

  LevelSetVolume(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void computeQpValue() override {};

protected:

  /// The accumulated volume to return as a PostprocessorValue
  Real _volume;

  /// The level set contour to consider for computing inside vs. outside of the volume
  const Real & _threshold;

  /// Flag for triggering the internal volume calculation
  const bool _inside;
};

#endif //LEVELSETVOLUME_H
