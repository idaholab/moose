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

#include "GenericConstantMaterial.h"

template<>
InputParameters validParams<GenericConstantMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::vector<std::string> >("prop_names", "The names of the properties this material will have");
  params.addParam<std::vector<Real> >("prop_values", "The values associated with the named properties");
  return params;
}

GenericConstantMaterial::GenericConstantMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _prop_names(getParam<std::vector<std::string> >("prop_names")),
    _prop_values(getParam<std::vector<Real> >("prop_values"))
{
  unsigned int num_names = _prop_names.size();
  unsigned int num_values = _prop_values.size();

  if(num_names != num_values)
    mooseError("Number of prop_names much match the number of prop_values for a GenericConstantMaterial!");

  _num_props = num_names;

  _properties.resize(num_names);

  for(unsigned int i=0; i<_num_props; i++)
    _properties[i] = &declareProperty<Real>(_prop_names[i]);
}

void
GenericConstantMaterial::computeQpProperties()
{
  for(unsigned int i=0; i<_num_props; i++)
    (*_properties[i])[_qp] = _prop_values[i];
}
