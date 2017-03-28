/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
  MaterialProperty<Real> & _density_old;
};

template <>
InputParameters validParams<Density>();

#endif // DENSITY_H
