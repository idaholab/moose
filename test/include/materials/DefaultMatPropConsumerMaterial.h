//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef DEFAULTMATPROPCONSUMERMATERIAL_H
#define DEFAULTMATPROPCONSUMERMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward declarations
class DefaultMatPropConsumerMaterial;

template <>
InputParameters validParams<DefaultMatPropConsumerMaterial>();

class DefaultMatPropConsumerMaterial : public DerivativeMaterialInterface<Material>
{
public:
  DefaultMatPropConsumerMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  std::string _prop_name;
  const MaterialProperty<Real> & _prop;
};

#endif // DEFAULTMATPROPCONSUMERMATERIAL_H
