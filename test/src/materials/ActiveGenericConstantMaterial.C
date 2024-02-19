//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ActiveGenericConstantMaterial.h"

registerMooseObject("MooseTestApp", ActiveGenericConstantMaterial);

InputParameters
ActiveGenericConstantMaterial::validParams()
{
  InputParameters params = GenericConstantMaterial::validParams();
  params.addClassDescription("To check whether properties are active.");
  return params;
}

ActiveGenericConstantMaterial::ActiveGenericConstantMaterial(const InputParameters & parameters)
  : GenericConstantMaterial(parameters)
{
}

void
ActiveGenericConstantMaterial::computeProperties()
{
  _console << "Element " << _current_elem->id() << " Subdomain " << _current_elem->subdomain_id()
           << ":" << std::endl;
  for (unsigned int i = 0; i < _num_props; i++)
  {
    auto id = _properties[i]->id();
    _console << " " << _prop_names[i] << " " << id << " " << isPropertyActive(id) << std::endl;
  }

  GenericConstantMaterial::computeProperties();
}

void
ActiveGenericConstantMaterial::computeQpProperties()
{
  for (unsigned int i = 0; i < _num_props; i++)
  {
    auto id = _properties[i]->id();
    if (isPropertyActive(id))
      (*_properties[i])[_qp] = _prop_values[i];
  }
}
