//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatSourceFromPowerDensity.h"
#include "HeatStructureInterface.h"
#include "HeatStructureCylindricalBase.h"

registerMooseObject("ThermalHydraulicsApp", HeatSourceFromPowerDensity);

InputParameters
HeatSourceFromPowerDensity::validParams()
{
  InputParameters params = HeatSourceBase::validParams();
  params.addRequiredParam<VariableName>("power_density", "Power density variable");
  params.addClassDescription("Heat source from power density");
  return params;
}

HeatSourceFromPowerDensity::HeatSourceFromPowerDensity(const InputParameters & parameters)
  : HeatSourceBase(parameters), _power_density_name(getParam<VariableName>("power_density"))
{
}

void
HeatSourceFromPowerDensity::addMooseObjects()
{
  /// The heat structure component we work with
  const HeatStructureInterface & hs = getComponent<HeatStructureInterface>("hs");
  const HeatStructureCylindricalBase * hs_cyl =
      dynamic_cast<const HeatStructureCylindricalBase *>(&hs);
  const bool is_cylindrical = hs_cyl != nullptr;

  {
    const std::string class_name = is_cylindrical ? "CoupledForceRZ" : "CoupledForce";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<SubdomainName>>("block") = _subdomain_names;
    pars.set<std::vector<VariableName>>("v") = std::vector<VariableName>(1, _power_density_name);
    if (is_cylindrical)
    {
      pars.set<Point>("axis_point") = hs_cyl->getPosition();
      pars.set<RealVectorValue>("axis_dir") = hs_cyl->getDirection();
      pars.set<Real>("offset") = hs_cyl->getInnerRadius() - hs_cyl->getAxialOffset();
    }
    std::string mon = genName(name(), "heat_src");
    getTHMProblem().addKernel(class_name, mon, pars);
  }
}
