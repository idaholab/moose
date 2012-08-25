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

#ifndef GENERICFUNCTIONMATERIAL_H
#define GENERICFUNCTIONMATERIAL_H

#include "Material.h"


//Forward Declarations
class GenericFunctionMaterial;

template<>
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
  GenericFunctionMaterial(const std::string & name, InputParameters parameters);

protected:
  virtual void computeQpProperties();

  std::vector<std::string> _prop_names;
  std::vector<FunctionName> _prop_values;

  unsigned int _num_props;

  std::vector<MaterialProperty<Real> *> _properties;
  std::vector<Function *> _functions;
};

#endif //GENERICFUNCTIONMATERIAL_H
