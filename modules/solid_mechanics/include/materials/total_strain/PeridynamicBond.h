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
  virtual void initQpStatefulProperties();

  MaterialProperty<Real> & _axial_force;
  MaterialProperty<Real> & _stiff_elem;
  MaterialProperty<Real> & _bond_status;
  MaterialProperty<Real> & _bond_status_old;
  MaterialProperty<Real> & _bond_stretch;
  MaterialProperty<Real> & _critical_stretch;
  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _bond_volume;

  const Real _youngs_modulus;
  const Real _poissons_ratio;
  const Real _MeshSpacing;
  const Real _ThicknessPerLayer;
  const Real _CriticalStretch;
  const Real _StandardDeviation;
  Real _MaterialRegion;
  Real _VolumePerNode;
  Real _lamda;

  int _callnum;

  bool _youngs_modulus_coupled;
  VariableValue & _youngs_modulus_var;

  bool _has_temp;
  VariableValue & _temp;
  const Real _t_ref;
  const Real _alpha;
  const Real _my_thermal_conductivity;
  Real _AvgArea;

  unsigned int _dim;
};

#endif //PERIDYNAMICBOND_H
