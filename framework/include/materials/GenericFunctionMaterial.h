//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GENERICFUNCTIONMATERIAL_H
#define GENERICFUNCTIONMATERIAL_H

#include "Material.h"

// Forward Declarations
class GenericFunctionMaterial;

template <>
InputParameters validParams<GenericFunctionMaterial>();

/**
 * This material automatically declares as material properties whatever is passed to it
 * through the parameters 'prop_names' and uses the Functions from 'prop_values' as the values
 * for those properties.
 *
 * This is not meant to be used in a production capacity... and instead is meant to be used
 * during development phases for ultimate flexibility.
 */
class GenericFunctionMaterial : public Material
{
public:
  GenericFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  std::vector<std::string> _prop_names;
  std::vector<FunctionName> _prop_values;

  unsigned int _num_props;

  std::vector<MaterialProperty<Real> *> _properties;
  std::vector<MaterialProperty<Real> *> _properties_old;
  std::vector<MaterialProperty<Real> *> _properties_older;
  std::vector<Function *> _functions;

private:
  /**
   * A helper method for evaluating the functions
   */
  void computeQpFunctions();

  /// Flag for calling declareProperyOld/Older
  bool _enable_stateful;
};

#endif // GENERICFUNCTIONMATERIAL_H
