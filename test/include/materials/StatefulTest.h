//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef STATEFULTEST_H
#define STATEFULTEST_H

#include "Material.h"

// Forward Declarations
class StatefulTest;

template <>
InputParameters validParams<StatefulTest>();

/**
 * Empty material for use in simple applications that don't need material properties.
 */
class StatefulTest : public Material
{
public:
  StatefulTest(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  // optional coupled variable
  const VariableValue * _coupled_val;

  std::vector<std::string> _prop_names;
  std::vector<Real> _prop_values;

  unsigned int _num_props;

  std::vector<MaterialProperty<Real> *> _properties;
  std::vector<const MaterialProperty<Real> *> _properties_old;
  std::vector<const MaterialProperty<Real> *> _properties_older;
};

#endif // STATEFULTEST_H
