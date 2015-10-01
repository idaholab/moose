/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "RadiationSource.h"

#include "Material.h"

template<>
InputParameters validParams<RadiationSource>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Specifies vacancy or interstitial source created due to radiation");
  MooseEnum defect_type("Vacancy=0 Interstitial", "Vacancy");
  params.addRequiredParam<MooseEnum>("defect_type", defect_type, "Specify the type of defect created due to radiation (vacancy or interstitial)");
  return params;
}
RadiationSource::RadiationSource(const InputParameters & parameters) :
    Kernel(parameters),
    _defect_type(getParam<MooseEnum>("defect_type")),
    _vacancy_increase(getMaterialProperty<Real>("vacancy_increase")),
    _interstitial_increase(getMaterialProperty<Real>("interstitial_increase"))
{
}

void
RadiationSource::subdomainSetup()
{
}

Real
RadiationSource::computeQpResidual()
{
  Real Pv = 0.0;

  switch (_defect_type)
  {
    case 0: //vacancy source
      Pv = _vacancy_increase[_qp];
      break;
    case 1: //Interstitial source
      Pv = _interstitial_increase[_qp];
      break;
    default:
      mooseError("Please provide valid defect type");
  }
  return - Pv;
}
