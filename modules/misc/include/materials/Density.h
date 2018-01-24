//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DENSITY_H
#define DENSITY_H

#include "Material.h"

/**
 * Compute density, which may changed based on a deforming mesh.
 */
class Density : public Material
{
public:
  Density(const InputParameters & params);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  bool _is_coupled;
  Moose::CoordinateSystemType _coord_system;
  std::vector<const VariableGradient *> _grad_disp;
  const VariableValue & _disp_r;

  const Real _orig_density;
  MaterialProperty<Real> & _density;
};

template <>
InputParameters validParams<Density>();

#endif // DENSITY_H
