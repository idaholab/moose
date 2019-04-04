//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEVALUEMATERIAL_H
#define INTERFACEVALUEMATERIAL_H

#include "InterfaceMaterial.h"
// #include "MaterialProperty.h"

// Forward Declarations
class InterfaceValueMaterial;

template <>
InputParameters validParams<InterfaceValueMaterial>();

/**
 * Interface material calculates a variable's jump value across an interface
 */
class InterfaceValueMaterial : public InterfaceMaterial
{
public:
  InterfaceValueMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;

  const std::string _mp_master_name;
  const std::string _mp_slave_name;
  const MaterialProperty<Real> & _mp_master;
  const MaterialProperty<Real> & _mp_slave;
  const VariableValue & _var_master;
  const VariableValue & _var_slave;
  /// the value type to be computed across the interface
  const MooseEnum _interface_value_type;
  const std::string _mp_out_base_name;
  const std::string _mp_var_out_base_name;
  MaterialProperty<Real> & _interface_value;
  MaterialProperty<Real> & _interface_value_2;
  const MaterialProperty<Real> & _interface_value_old;
  const MaterialProperty<Real> & _interface_value_2_old;

  // virtual Real computeInterfaceValueType(const Real /*value_master*/, const Real
  // /*value_slave*/);
};

#endif // INTERFACEVALUEMATERIAL_H
