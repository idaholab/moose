#ifndef ELASTICENERGYMATERIAL_H
#define ELASTICENERGYMATERIAL_H

#include "DerivativeBaseMaterial.h"

// Forward Declaration
class ElasticEnergyMaterial;
class RankTwoTensor;

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
  virtual Real computeF();
  virtual Real computeDF(unsigned int);
  virtual Real computeD2F(unsigned int, unsigned int);

  /// any number of args is accepted.
  virtual unsigned int expectedNumArgs() { return _nargs; }

  std::string _base_name;

  // mechanics properties
  const MaterialProperty<RankTwoTensor> & _stress;
  std::vector<const MaterialProperty<RankTwoTensor> *> _dstress;
  std::vector<std::vector<const MaterialProperty<RankTwoTensor> *> > _d2stress;
  const MaterialProperty<RankTwoTensor> & _strain;
  std::vector<const MaterialProperty<RankTwoTensor> *> _dstrain;
  std::vector<std::vector<const MaterialProperty<RankTwoTensor> *> > _d2strain;
};

#endif //ELASTICENERGYMATERIAL_H
