/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#ifndef THROW_MATERIAL_H
#define THROW_MATERIAL_H

#include "Material.h"

class ThrowMaterial;

template <>
InputParameters validParams<ThrowMaterial>();

/**
 * ThrowMaterial throws a MooseException when certain conditions are
 * met.
 */
class ThrowMaterial : public Material
{
public:
  ThrowMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// The MaterialProperty value we are responsible for computing
  MaterialProperty<Real> & _prop_value;

  /// The value of our MaterialProperty depends on the value of a coupled variable
  const VariableValue & _coupled_var;

  static bool _has_thrown;
};

#endif
