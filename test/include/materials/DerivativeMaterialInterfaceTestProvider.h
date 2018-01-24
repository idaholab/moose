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
