#include "RandomMaterial.h"

template <>
InputParameters
validParams<RandomMaterial>()
{
  InputParameters params = validParams<Material>();
  return params;
}

RandomMaterial::RandomMaterial(const InputParameters & parameters)
  : Material(parameters),
    _rand_real(declareProperty<Real>("rand_real")),
    _rand_long(declareProperty<unsigned long>("rand_long"))
{
  setRandomResetFrequency(EXEC_TIMESTEP_END);
}

void
RandomMaterial::computeQpProperties()
{
  _rand_real[_qp] = getRandomReal();
  _rand_long[_qp] = getRandomLong();
}
