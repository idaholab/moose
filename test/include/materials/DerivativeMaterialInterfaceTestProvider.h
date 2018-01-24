//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DERIVATIVEMATERIALINTERFACETESTPROVIDER_H
#define DERIVATIVEMATERIALINTERFACETESTPROVIDER_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

class DerivativeMaterialInterfaceTestProvider;

template <>
InputParameters validParams<DerivativeMaterialInterfaceTestProvider>();

/**
 * Test class that provides a few material properties through DerivativeMaterialInterface
 */
class DerivativeMaterialInterfaceTestProvider : public DerivativeMaterialInterface<Material>
{
public:
  DerivativeMaterialInterfaceTestProvider(const InputParameters & parameters);

  virtual void computeQpProperties();

protected:
  MaterialProperty<Real> &_prop1, &_prop2, &_prop3, &_prop4, &_prop5;
  MaterialProperty<dof_id_type> & _prop6;
};

#endif // DERIVATIVEMATERIALINTERFACETESTPROVIDER_H
