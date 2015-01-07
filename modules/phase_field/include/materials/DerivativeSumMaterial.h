#ifndef DERIVATIVESUMMATERIAL_H
#define DERIVATIVESUMMATERIAL_H

#include "DerivativeBaseMaterial.h"

class DerivativeSumMaterial;

template<>
InputParameters validParams<DerivativeSumMaterial>();

class DerivativeSumMaterial : public DerivativeBaseMaterial
{
public:
  DerivativeSumMaterial(const std::string & name, InputParameters parameters);

protected:
  virtual void computeProperties();

  std::vector<std::string> _sum_materials;
  unsigned int _num_materials;

  /// Function values of the summands.
  std::vector<const MaterialProperty<Real> *> _summand_F;

  /// Derivatives of the summands with respect to arg[i]
  std::vector<std::vector<const MaterialProperty<Real> *> > _summand_dF;

  /// Second derivatives of the summands.
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *> > > _summand_d2F;

  /// Third derivatives of the summands.
  std::vector<std::vector<std::vector<std::vector<const MaterialProperty<Real> *> > > > _summand_d3F;
};

#endif //DERIVATIVESUMMATERIAL_H
