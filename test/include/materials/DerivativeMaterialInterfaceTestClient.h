#ifndef DERIVATIVEMATERIALINTERFACETESTCLIENT_H
#define DERIVATIVEMATERIALINTERFACETESTCLIENT_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

class DerivativeMaterialInterfaceTestClient;

template<>
InputParameters validParams<DerivativeMaterialInterfaceTestClient>();

/**
 * Test class that provides a few material properties through DerivativeMaterialInterface
 */
class DerivativeMaterialInterfaceTestClient : public DerivativeMaterialInterface<Material>
{
public:
  DerivativeMaterialInterfaceTestClient(const std::string & name, InputParameters parameters);

protected:
  void computeProperties();

  const MaterialProperty<Real> & _prop0, & _prop1, & _prop2, & _prop3, & _prop4, & _prop5;
};

#endif //DERIVATIVEMATERIALINTERFACETESTCLIENT_H
