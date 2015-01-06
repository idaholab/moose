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
  virtual Real computeF();
  virtual Real computeDF(unsigned int);
  virtual Real computeD2F(unsigned int, unsigned int);
  virtual Real computeD3F(unsigned int, unsigned int, unsigned int);

  std::vector<std::string> _f_names;
  unsigned int _num_materials;

  /// Function values of the summands.
  std::vector<const MaterialProperty<Real> *> _prop_F;

  /// Derivatives of the summands with respect to arg[i]
  std::vector<std::vector<const MaterialProperty<Real> *> > _prop_dF;

  /// Second derivatives of the summands.
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *> > > _prop_d2F;

  /// Third derivatives of the summands.
  std::vector<std::vector<std::vector<std::vector<const MaterialProperty<Real> *> > > > _prop_d3F;
};

#endif //DERIVATIVESUMMATERIAL_H
