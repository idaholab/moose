/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LINEARISOTROPICMATERIALPD_H
#define LINEARISOTROPICMATERIALPD_H

#include "Material.h"

//Forward Declarations
class LinearIsotropicMaterialPD;

template<>
InputParameters validParams<LinearIsotropicMaterialPD>();

/**
 * LinearIsotropic material for use in simple applications that don't need material properties.
 */
class LinearIsotropicMaterialPD : public Material
{
public:
  LinearIsotropicMaterialPD(const InputParameters & parameters);
  virtual ~LinearIsotropicMaterialPD();

protected:

  MooseVariable * _disp_x_var;
  MooseVariable * _disp_y_var;
  MooseVariable * _disp_z_var;
  MooseVariable * _temp_var;


  virtual void computeProperties();
  virtual void initQpStatefulProperties();

  MaterialProperty<Real> & _bond_force;
  MaterialProperty<Real> & _bond_force_dif;
  MaterialProperty<Real> & _bond_status;
  MaterialProperty<Real> & _bond_status_old;
  MaterialProperty<Real> & _critical_stretch;

  const int _pddim;
  const Real _youngs_modulus;
  const Real _poissons_ratio;
  const Real _mesh_spacing;
  const Real _domain_thickness;
  const Real _my_critical_stretch;
  const Real _standard_deviation;

  bool _has_temp;
  const Real _temp_ref;
  const Real _thermal_expansion;
};

#endif //LINEARISOTROPICMATERIALPD_H
