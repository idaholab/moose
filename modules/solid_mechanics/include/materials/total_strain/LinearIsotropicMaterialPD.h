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

class LinearIsotropicMaterialPD : public Material
{
public:
  LinearIsotropicMaterialPD(const InputParameters & parameters);
  virtual ~LinearIsotropicMaterialPD();

protected:

  MooseVariable * _disp_x_var;
  MooseVariable * _disp_y_var;
  MooseVariable * _disp_z_var;

  virtual void computeProperties();
  virtual void initQpStatefulProperties();

  MaterialProperty<Real> & _bond_force;
  MaterialProperty<Real> & _bond_force_dif_disp;
  MaterialProperty<Real> & _bond_force_dif_temp;
  MaterialProperty<Real> & _bond_mechanic_strain;
  MaterialProperty<Real> & _bond_critical_strain;
  MaterialProperty<Real> & _bond_critical_strain_old;

  const int _pddim;
  const Real _youngs_modulus;
  const Real _poissons_ratio;
  const Real _mesh_spacing;
  const Real _critical_strain;
  const Real _standard_deviation;

  bool _is_plane_strain;
  bool _is_vary_stiffness;

  bool _has_temp;
  VariableValue & _temp;
  const Real _temp_ref;
  const Real _thermal_expansion;
};

#endif //LINEARISOTROPICMATERIALPD_H
