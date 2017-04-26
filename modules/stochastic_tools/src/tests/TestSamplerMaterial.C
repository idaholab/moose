/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "TestSamplerMaterial.h"
#include <algorithm>

// libMesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<TestSamplerMaterial>()
{
  InputParameters params = validParams<GenericConstantMaterial>();
  params.addRequiredParam<SamplerName>(
      "sampler", "The sampler which supplies the perturbation factor for the material properties");
  return params;
}

TestSamplerMaterial::TestSamplerMaterial(const InputParameters & parameters)
  : GenericConstantMaterial(parameters), SamplerInterface(this), _sampler(getSampler("sampler"))
{
  std::vector<std::string> perturbed_params;
  std::vector<std::string> perturbed_prop;
  perturbed_params = _sampler.getSampledVariableNames();
  for (unsigned int i = 0; i < _num_props; ++i)
  {
    if (std::find(perturbed_params.begin(), perturbed_params.end(), _prop_names[i]) !=
        perturbed_params.end())
    {
      perturbed_prop.clear();
      perturbed_prop.push_back(_prop_names[i]);
      _prop_values[i] *= _sampler.getSampledValues(perturbed_prop)[0];
    }
  }
}
