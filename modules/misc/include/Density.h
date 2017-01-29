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
  Density( const InputParameters & params);

  virtual void initStatefulProperties(unsigned n_points);

protected:
  virtual void computeProperties();

  const bool _is_coupled;
  const bool _is_RZ;
  const bool _is_SphericalR;
  const VariableGradient & _grad_disp_x;
  const VariableGradient & _grad_disp_y;
  const VariableGradient & _grad_disp_z;
  const VariableValue & _disp_r;

  const Real _orig_density;
  MaterialProperty<Real> & _orig_density_prop;
  MaterialProperty<Real> & _density;
  MaterialProperty<Real> & _density_old;
};

template<>
InputParameters validParams<Density>();

#endif // DENSITY_H
