/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef MAXIMUMNORMALSEPARATION_H
#define MAXIMUMNORMALSEPARATION_H

#include "Material.h"

class MaximumNormalSeparation;

template <>
InputParameters validParams<MaximumNormalSeparation>();

/**
 *
 */
class MaximumNormalSeparation : public Material
{
public:
  MaximumNormalSeparation(const InputParameters & parameters);

protected:
  virtual void resetQpProperties() override;
  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;

  const std::string _base_name;
  MaterialProperty<Real> & _max_normal_separation;
  const MaterialProperty<Real> & _max_normal_separation_old;

  const VariableValue & _disp_x;
  const VariableValue & _disp_x_neighbor;
  const VariableValue & _disp_y;
  const VariableValue & _disp_y_neighbor;
};

#endif // MAXIMUMNORMALSEPARATION_H
