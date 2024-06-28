#include "CustomFormulation.h"

registerMooseObject("PlatypusApp", CustomFormulation);

InputParameters
CustomFormulation::validParams()
{
  InputParameters params = MFEMFormulation::validParams();
  return params;
}

CustomFormulation::CustomFormulation(const InputParameters & parameters)
  : MFEMFormulation(parameters),
    _formulation(std::make_shared<platypus::TimeDomainEquationSystemProblemBuilder>())
{
}
