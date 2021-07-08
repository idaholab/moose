/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#pragma once

#include "Material.h"

/**
 *
 */
class MaximumNormalSeparation : public Material
{
public:
  static InputParameters validParams();

  MaximumNormalSeparation(const InputParameters & parameters);

protected:
  virtual void resetQpProperties() override;
  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;

  const std::string _base_name;
  MaterialProperty<Real> & _max_normal_separation;
  const MaterialProperty<Real> & _max_normal_separation_old;

  const std::vector<const VariableValue *> _disp;
  const std::vector<const VariableValue *> _disp_neighbor;
};
