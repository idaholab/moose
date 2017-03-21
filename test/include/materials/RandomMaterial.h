#ifndef RANDOMMATERIAL_H
#define RANDOMMATERIAL_H

#include "Material.h"

class RandomMaterial;

template <>
InputParameters validParams<RandomMaterial>();

class RandomMaterial : public Material
{
public:
  RandomMaterial(const InputParameters & parameters);
  virtual void computeQpProperties();

protected:
  MaterialProperty<Real> & _rand_real;
  MaterialProperty<unsigned long> & _rand_long;
};

#endif // RANDOMMATERIAL_H
