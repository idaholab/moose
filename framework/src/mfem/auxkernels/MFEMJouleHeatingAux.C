#include "MFEMJouleHeatingAux.h"

registerMooseObject("PlatypusApp", MFEMJouleHeatingAux);

InputParameters
MFEMJouleHeatingAux::validParams()
{
  InputParameters params = MFEMAuxSolver::validParams();

  return params;
}

MFEMJouleHeatingAux::MFEMJouleHeatingAux(const InputParameters & parameters)
  : MFEMAuxSolver(parameters),
    joule_heating_params({{"CoupledVariableName", std::string("electric_field")},
                          {"ConductivityCoefName", std::string("electrical_conductivity")},
                          {"JouleHeatingVarName", std::string("joule_heating")}}),
    joule_heating_aux{std::make_shared<JouleHeatingCoefficient>(joule_heating_params)}
{
}

void
MFEMJouleHeatingAux::storeCoefficients(hephaestus::Coefficients & coefficients)
{
  coefficients._scalars.Register("JouleHeating", joule_heating_aux);
}

MFEMJouleHeatingAux::~MFEMJouleHeatingAux() {}
