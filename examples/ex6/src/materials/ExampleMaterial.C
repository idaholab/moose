#include "ExampleMaterial.h"

template<>
InputParameters validParams<ExampleMaterial>()
{
  InputParameters params;
  params.addParam<Real>("diffusivity", 1.0, "The Diffusivity");
  return params;
}

ExampleMaterial::ExampleMaterial(std::string name,
                                 InputParameters parameters,
                                 unsigned int block_id,
                                 std::vector<std::string> coupled_to,
                                 std::vector<std::string> coupled_as)
  :Material(name,parameters,block_id,coupled_to,coupled_as),
   
   // Get a parameter value for the diffusivity
   _input_diffusivity(parameters.get<Real>("diffusivity")),

   // Declare that this material is going to have a Real
   // valued property named "diffusivity" that Kernels can use.
   _diffusivity(declareRealProperty("diffusivity"))
{}

void
ExampleMaterial::computeProperties()
{
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
    _diffusivity[qp] = _input_diffusivity;
  }
}
