#include "TransientCustomFormulation.h"

registerMooseObject("PlatypusApp", TransientCustomFormulation);

InputParameters
TransientCustomFormulation::validParams()
{
  InputParameters params = MFEMFormulation::validParams();
  params.addClassDescription(
      "Add an MFEM Formulation suitable for use with Transient executioners to the problem.");

  return params;
}

TransientCustomFormulation::TransientCustomFormulation(const InputParameters & parameters)
  : MFEMFormulation(parameters),
    _formulation(std::make_shared<platypus::TimeDomainEquationSystemProblemBuilder>())
{
}
