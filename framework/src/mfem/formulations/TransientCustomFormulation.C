#include "TransientCustomFormulation.h"

registerMooseObject("PlatypusApp", TransientCustomFormulation);

InputParameters
TransientCustomFormulation::validParams()
{
  InputParameters params = MFEMFormulation::validParams();
  return params;
}

TransientCustomFormulation::TransientCustomFormulation(const InputParameters & parameters)
  : MFEMFormulation(parameters),
    _formulation(std::make_shared<platypus::TimeDomainEquationSystemProblemBuilder>())
{
}
