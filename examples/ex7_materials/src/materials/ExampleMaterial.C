#include "ExampleMaterial.h"

template<>
InputParameters validParams<ExampleMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<Real>("diffusivity", 1.0, "The Diffusivity");
  return params;
}

ExampleMaterial::ExampleMaterial(std::string name,
                                 MooseSystem & moose_system,
                                 InputParameters parameters)
  :Material(name, moose_system, parameters),
   
   // Get a parameter value for the diffusivity
   _input_diffusivity(parameters.get<Real>("diffusivity")),

   // Declare that this material is going to have a Real
   // valued property named "diffusivity" that Kernels can use.
   _diffusivity(declareRealProperty("diffusivity"))
{}

void
ExampleMaterial::computeProperties()
{
  for(unsigned int qp=0; qp<_n_qpoints; qp++)
  {
    _diffusivity[qp] = _input_diffusivity;
  }
}
