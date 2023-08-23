#include "AnisotropyInterfaceMaterialAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "Conversion.h"

registerMooseAction("PhaseFieldApp", AnisotropyInterfaceMaterialAction, "add_material");

InputParameters
AnisotropyInterfaceMaterialAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Set up materials for the InterfaceOrientationMultiphaseMaterial "
      "for N number of phases with anisotropic gradient energy coefficients.");
  params.addRequiredParam<std::vector<VariableName>>("etas", "List of order parameters variables");
  params.addParam<MaterialPropertyName>(
      "kappa_name", "kappa", "speficies the base name for the gradient energy coefficient");
  params.addParam<MaterialPropertyName>("dkappadgrad_eta_name",
                                        "dkappadgrad",
                                        "specifies the base name for the first derivative of "
                                        "gradient energy coefficient with respect to grad etas");
  params.addParam<MaterialPropertyName>("d2kappadgrad_eta_name",
                                        "d2kappadgrad_",
                                        "specifies the base name for the second derivative of "
                                        "gradient energy coefficient with respect to grad etas");
  params.addParam<Real>(
      "anisotropy_strength", 0.04, "Strength of the anisotropy (typically < 0.05)");
  params.addParam<unsigned int>("mode_number", 4, "Mode number for anisotropy");
  params.addParam<std::vector<Real>>(
      "reference_angle",
      "Reference angle for defining anisotropy in degrees with respect to the liquid");
  params.addParam<Real>("kappa_bar", 0.1125, "Average value of the interface parameter kappa");
  params.addParam<std::vector<std::string>>(
      "output_kappa_name", "Name of material property for the combined kappa and kappa gradients");
  return params;
}

AnisotropyInterfaceMaterialAction::AnisotropyInterfaceMaterialAction(
    const InputParameters & parameters)
  : Action(parameters)
{
}

void
AnisotropyInterfaceMaterialAction::act()
{
  const auto etas = getParam<std::vector<VariableName>>("etas");
  const auto kappa_name = getParam<MaterialPropertyName>("kappa_name");
  const auto dkappadgrad_eta_name = getParam<MaterialPropertyName>("dkappadgrad_eta_name");
  const auto d2kappadgrad_eta_name = getParam<MaterialPropertyName>("d2kappadgrad_eta_name");
  const auto anisotropy_strength = getParam<Real>("anisotropy_strength");
  const auto mode_number = getParam<unsigned int>("mode_number");
  const auto reference_angle = getParam<std::vector<Real>>("reference_angle");
  const auto kappa_bar = getParam<Real>("kappa_bar");
  const auto output_kappa_name = getParam<std::vector<std::string>>("output_kappa_name");

  // Size definitions for etas
  unsigned int n_etas = etas.size();
  std::string material_name;
  VariableName etaa_name;
  VariableName etab_name;
  Real angle_a;
  Real angle_b;
  std::vector<VariableName> etaa;
  etaa.resize(1);
  std::vector<VariableName> etab;
  etab.resize(1);
  for (unsigned int i = 0; i < n_etas; ++i)
  {
    etaa_name = etas[i];
    etaa[0] = etas[i];
    angle_a = reference_angle[i];
    for (unsigned int j = 0; j < n_etas; ++j)
    {
      if (i != j)
      {
        etab_name = etas[j];
        etab[0] = etas[j];
        angle_b = reference_angle[j];

        InputParameters params = _factory.getValidParams("InterfaceOrientationMultiphaseMaterial");
        params.set<std::vector<VariableName>>("etaa") = etaa;
        params.set<std::vector<VariableName>>("etab") = etab;
        params.set<MaterialPropertyName>("kappa_name") =
            kappa_name + '_' + etaa_name + '_' + etab_name;
        params.set<MaterialPropertyName>("dkappadgrad_etaa_name") =
            dkappadgrad_eta_name + '_' + etaa_name + '_' + etab_name;
        params.set<MaterialPropertyName>("d2kappadgrad_etaa_name") =
            d2kappadgrad_eta_name + '_' + etaa_name + '_' + etab_name;
        params.set<Real>("anisotropy_strength") = anisotropy_strength;
        params.set<unsigned int>("mode_number") = mode_number;
        params.set<Real>("kappa_bar") = kappa_bar;
        params.set<Real>("reference_angle") = angle_a - angle_b;
        material_name = "kappa_" + etaa_name + "_" + etab_name;
        _problem->addMaterial("InterfaceOrientationMultiphaseMaterial", material_name, params);
      }
    }
  }
}
