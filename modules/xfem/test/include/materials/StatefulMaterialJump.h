/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATEFULMATERIALJUMP_H
#define STATEFULMATERIALJUMP_H

#include "Material.h"

class StatefulMaterialJump;

template <>
InputParameters validParams<StatefulMaterialJump>();

/**
 *
 */
class StatefulMaterialJump : public Material
{
public:
  StatefulMaterialJump(const InputParameters & parameters);

protected:
  virtual void resetQpProperties() override;
  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;

  MaterialProperty<Real> & _prop;
  const MaterialProperty<Real> & _prop_old;

  const VariableValue & _u;
  const VariableValue & _u_neighbor;
};

#endif // STATEFULMATERIALJUMP_H
