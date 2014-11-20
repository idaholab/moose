#ifndef ELASTICENERGYMATERIAL_H
#define ELASTICENERGYMATERIAL_H

#include "DerivativeBaseMaterial.h"
#include "RankTwoTensor.h"

// Forward Declaration
class ElasticEnergyMaterial;

template<>
InputParameters validParams<DerivativeBaseMaterial>();

/**
 * Material class to compute the elastic free energy and its derivatives
 */
class ElasticEnergyMaterial : public DerivativeBaseMaterial
{
public:
  ElasticEnergyMaterial(const std::string & name, InputParameters parameters);

protected:
  std::string _strain_name;
  MaterialProperty<RankTwoTensor> & _strain;
};

#endif //ELASTICENERGYMATERIAL_H
