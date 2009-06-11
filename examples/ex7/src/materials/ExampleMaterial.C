#include "ExampleMaterial.h"

template<>
Parameters valid_params<ExampleMaterial>()
{
  Parameters params;
  params.set<Real>("diffusivity")=1.0;
  params.set<Real>("time_coefficient")=1.0;
  return params;
}

ExampleMaterial::ExampleMaterial(std::string name,
                                 Parameters parameters,
                                 unsigned int block_id,
                                 std::vector<std::string> coupled_to,
                                 std::vector<std::string> coupled_as)
  :Material(name,parameters,block_id,coupled_to,coupled_as),
   
   // Get a parameter value for the diffusivity
   _input_diffusivity(parameters.get<Real>("diffusivity")),

   // Get a parameter value for the time_coefficient
   _input_time_coefficient(parameters.get<Real>("time_coefficient")),

   // Declare that this material is going to have a Real
   // valued property named "diffusivity" that Kernels can use.
   _diffusivity(declareRealProperty("diffusivity")),

   // Declare that this material is going to have a Real
   // valued property named "time_coefficient" that Kernels can use.
   _time_coefficient(declareRealProperty("time_coefficient"))
{}

void
ExampleMaterial::computeProperties()
{
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
    _diffusivity[qp] = _input_diffusivity;
    _time_coefficient[qp] = _input_time_coefficient;
  }
}
