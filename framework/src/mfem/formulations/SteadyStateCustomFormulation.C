#include "SteadyStateCustomFormulation.h"

registerMooseObject("PlatypusApp", SteadyStateCustomFormulation);

InputParameters
SteadyStateCustomFormulation::validParams()
{
  InputParameters params = MFEMFormulation::validParams();
  params.addClassDescription(
      "MFEM Formulation suitable for use with Steady executioners to the problem.");

  return params;
}

SteadyStateCustomFormulation::SteadyStateCustomFormulation(const InputParameters & parameters)
  : MFEMFormulation(parameters),
    _formulation(std::make_shared<platypus::SteadyStateEquationSystemProblemBuilder>())
{
}
