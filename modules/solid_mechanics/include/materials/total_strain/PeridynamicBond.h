/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PERIDYNAMICBOND_H
#define PERIDYNAMICBOND_H

#include "Material.h"

//Forward Declarations
class PeridynamicBond;

template<>
InputParameters validParams<PeridynamicBond>();

/**
 * LinearIsotropic material for use in simple applications that don't need material properties.
 */
class PeridynamicBond : public Material
{
public:
  PeridynamicBond(const std::string & name,
                InputParameters parameters);

  virtual ~PeridynamicBond();

protected:

  MooseVariable * _disp_x_var;
  MooseVariable * _disp_y_var;
  MooseVariable * _disp_z_var;


  virtual void computeProperties();

  MaterialProperty<Real> & _axial_force;
  MaterialProperty<Real> & _stiff_elem;
  Real _youngs_modulus;
  Real _poissons_ratio;
  Real _MeshSpacing;
  Real _MaterialRegion;
  Real _ThicknessPerLayer;
  Real _VolumePerNode;
  Real _lamda;
  bool _youngs_modulus_coupled;
  VariableValue & _youngs_modulus_var;

  bool _has_temp;
  VariableValue & _temp;
  Real _t_ref;
  Real _alpha;

  unsigned int _dim;
};

#endif //PERIDYNAMICBOND_H
