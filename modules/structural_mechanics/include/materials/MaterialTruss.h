/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MATERIALTRUSS_H
#define MATERIALTRUSS_H

#include "Material.h"

//Forward Declarations
class MaterialTruss;

template<>
InputParameters validParams<MaterialTruss>();

/**
 * LinearIsotropic material for use in simple applications that don't need material properties.
 */
class MaterialTruss : public Material
{
public:
  MaterialTruss(const InputParameters & parameters);

  virtual ~MaterialTruss();

protected:

  MooseVariable * _disp_x_var;
  MooseVariable * _disp_y_var;
  MooseVariable * _disp_z_var;


  virtual void computeProperties();

  MaterialProperty<Real> & _axial_stress;
  MaterialProperty<Real> & _e_over_l;
  Real _youngs_modulus;
  bool _youngs_modulus_coupled;
  const VariableValue & _youngs_modulus_var;

  bool _has_temp;
  const VariableValue & _temp;
  Real _t_ref;
  Real _alpha;

  unsigned int _dim;
};

#endif //MATERIALTRUSS_H
