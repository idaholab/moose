#include "DerivativeMaterialInterfaceTestProvider.h"

template<>
InputParameters validParams<DerivativeMaterialInterfaceTestProvider>()
{
  InputParameters params = validParams<Material>();
  return params;
}

DerivativeMaterialInterfaceTestProvider::DerivativeMaterialInterfaceTestProvider(const std::string & name,
                                                                                 InputParameters parameters) :
    DerivativeMaterialInterface<Material>(name, parameters),
    _prop1(declareProperty<Real>(propertyNameFirst("prop","a"))),
    _prop2(declareProperty<Real>(propertyNameFirst("prop","b"))),
    _prop3(declareProperty<Real>(propertyNameSecond("prop","b", "a"))),
    _prop4(declareProperty<Real>(propertyNameSecond("prop","a", "c"))),
    _prop5(declareProperty<Real>(propertyNameThird("prop","b", "c", "a")))
{
}

void
DerivativeMaterialInterfaceTestProvider::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _prop1[_qp] = 1.0;
    _prop2[_qp] = 2.0;
    _prop3[_qp] = 3.0;
    _prop4[_qp] = 4.0;
    _prop5[_qp] = 5.0;
  }
}
