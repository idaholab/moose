/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TRUSSMATERIAL_H
#define TRUSSMATERIAL_H

#include "Material.h"

// Forward Declarations
class TrussMaterial;

template <>
InputParameters validParams<TrussMaterial>();

class TrussMaterial : public Material
{
public:
  TrussMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeProperties();

  virtual void computeQpStrain() = 0;
  virtual void computeQpStress() = 0;

  std::vector<MooseVariable *> _disp_var;

  const std::string _base_name;

  unsigned int _ndisp;
  const VariableValue & _youngs_modulus;

  MaterialProperty<Real> & _total_stretch;
  MaterialProperty<Real> & _elastic_stretch;
  MaterialProperty<Real> & _axial_stress;
  MaterialProperty<Real> & _e_over_l;

  Real _origin_length;
  Real _current_length;
};

#endif // TRUSSMATERIAL_H
