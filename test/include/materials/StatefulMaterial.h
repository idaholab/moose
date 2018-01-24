//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef STATEFULMATERIAL_H
#define STATEFULMATERIAL_H

#include "Material.h"

// Forward Declarations
class StatefulMaterial;

template <>
InputParameters validParams<StatefulMaterial>();

/**
 * Stateful material class that defines a few properties.
 */
class StatefulMaterial : public Material
{
public:
  StatefulMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

private:
  Real _initial_diffusivity;

  /**
   * Create two MooseArray Refs to hold the current
   * and previous material properties respectively
   */
  MaterialProperty<Real> & _diffusivity;
  const MaterialProperty<Real> & _diffusivity_old;
};

#endif // STATEFULMATERIAL_H
