#include "ExampleMaterial.h"

template<>
Parameters valid_params<ExampleMaterial>()
{
  Parameters params;
  params.set<Real>("diffusivity")=1.0;
  return params;
}

void
ExampleMaterial::computeProperties()
{
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
    _diffusivity[qp] = _input_diffusivity;
  }
}
