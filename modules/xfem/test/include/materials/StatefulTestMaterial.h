/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATEFULTESTMATERIAL_H
#define STATEFULTESTMATERIAL_H

#include "Material.h"

class StatefulTestMaterial;

template <>
InputParameters validParams<StatefulTestMaterial>();

/**
 *
 */
class StatefulTestMaterial : public Material
{
public:
  StatefulTestMaterial(const InputParameters & parameters);

protected:
  virtual void resetQpProperties() override;
  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;

  MaterialProperty<Real> & _prop;
  const MaterialProperty<Real> & _prop_old;
  const MaterialProperty<Real> & _prop1_old;
};

#endif // STATEFULTESTMATERIAL_H
