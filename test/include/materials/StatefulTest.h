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
