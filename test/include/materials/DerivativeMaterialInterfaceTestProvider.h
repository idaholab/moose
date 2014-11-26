#ifndef DERIVATIVEMATERIALINTERFACETESTPROVIDER_H
#define DERIVATIVEMATERIALINTERFACETESTPROVIDER_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

class DerivativeMaterialInterfaceTestProvider;

template<>
InputParameters validParams<DerivativeMaterialInterfaceTestProvider>();

/**
 * Test class that provides a few material properties through DerivativeMaterialInterface
 */
class DerivativeMaterialInterfaceTestProvider : public DerivativeMaterialInterface<Material>
{
public:
  DerivativeMaterialInterfaceTestProvider(const std::string & name, InputParameters parameters);

protected:
  void computeProperties();

  MaterialProperty<Real> & _prop1, & _prop2, & _prop3, & _prop4, & _prop5;
};

#endif //DERIVATIVEMATERIALINTERFACETESTPROVIDER_H
