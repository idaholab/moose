#include "CustomFormulation.h"

registerMooseObject("PlatypusApp", CustomFormulation);

InputParameters
CustomFormulation::validParams()
{
  InputParameters params = MFEMFormulation::validParams();
  return params;
}

CustomFormulation::CustomFormulation(const InputParameters & parameters)
  : MFEMFormulation(parameters)
{
  formulation = std::make_shared<hephaestus::TimeDomainEMFormulation>();
}

CustomFormulation::~CustomFormulation() {}
