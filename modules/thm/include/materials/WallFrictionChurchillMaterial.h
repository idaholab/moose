#ifndef WALLFRICTIONCHURCHILLMATERIAL_H
#define WALLFRICTIONCHURCHILLMATERIAL_H

#include "WallFriction3EqnBaseMaterial.h"

class WallFrictionChurchillMaterial;

template <>
InputParameters validParams<WallFrictionChurchillMaterial>();

/**
 * Computes drag coefficient using the Churchill formula for Fanning friction factor
 */
class WallFrictionChurchillMaterial : public WallFriction3EqnBaseMaterial
{
public:
  WallFrictionChurchillMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
};

#endif // WALLFRICTIONCHURCHILLMATERIAL_H
