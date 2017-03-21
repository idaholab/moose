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
#ifndef DERIVATIVEMATERIALINTERFACETESTCLIENT_H
#define DERIVATIVEMATERIALINTERFACETESTCLIENT_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

class DerivativeMaterialInterfaceTestClient;

template <>
InputParameters validParams<DerivativeMaterialInterfaceTestClient>();

/**
 * Test class that provides a few material properties through DerivativeMaterialInterface
 */
class DerivativeMaterialInterfaceTestClient : public DerivativeMaterialInterface<Material>
{
public:
  DerivativeMaterialInterfaceTestClient(const InputParameters & parameters);

  virtual void initialSetup();
  virtual void computeQpProperties();

protected:
  MaterialPropertyName _prop_name;
  bool _by_name;
  const MaterialProperty<Real> &_prop0, &_prop1, &_prop2, &_prop3, &_prop4, &_prop5;
  const MaterialProperty<dof_id_type> & _prop6;
};

#endif // DERIVATIVEMATERIALINTERFACETESTCLIENT_H
