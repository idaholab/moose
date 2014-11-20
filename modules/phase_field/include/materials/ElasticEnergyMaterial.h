#ifndef ELASTICENERGYMATERIAL_H
#define ELASTICENERGYMATERIAL_H

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
};

#endif //ELASTICENERGYMATERIAL_H
