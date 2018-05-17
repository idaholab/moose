//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EXCEPTIONMATERIAL_H
#define EXCEPTIONMATERIAL_H

#include "Material.h"

class ExceptionMaterial;

template <>
InputParameters validParams<ExceptionMaterial>();

/**
 * ExceptionMaterial throws a MooseException when certain conditions are
 * met.
 */
class ExceptionMaterial : public Material
{
public:
  ExceptionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// The MaterialProperty value we are responsible for computing
  MaterialProperty<Real> & _prop_value;

  /// The value of our MaterialProperty depends on the value of a coupled variable
  const VariableValue & _coupled_var;

  // The rank to isolate the exception to if valid
  const processor_id_type _rank;

  /// throw only once
  bool _has_thrown;
};

#endif // EXCEPTIONMATERIAL_H
