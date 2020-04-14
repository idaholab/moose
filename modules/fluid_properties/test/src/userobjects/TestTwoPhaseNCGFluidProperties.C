//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestTwoPhaseNCGFluidProperties.h"

registerMooseObject("FluidPropertiesTestApp", TestTwoPhaseNCGFluidProperties);

InputParameters
TestTwoPhaseNCGFluidProperties::validParams()
{
  InputParameters params = TwoPhaseNCGFluidProperties::validParams();

  params.addClassDescription("Test 2-phase NCG fluid properties");

  params.addRequiredParam<std::vector<UserObjectName>>(
      "fp_ncgs", "Name of fluid properties user object(s) for non-condensable gases");

  return params;
}

TestTwoPhaseNCGFluidProperties::TestTwoPhaseNCGFluidProperties(const InputParameters & parameters)
  : TwoPhaseNCGFluidProperties(parameters)
{
  // create liquid and vapor fluid properties
  if (_tid == 0)
  {
    {
      const std::string class_name = "IdealGasFluidProperties";
      InputParameters params = _app.getFactory().getValidParams(class_name);
      params.set<Real>("gamma") = 1.4;
      params.set<Real>("molar_mass") = 0.030794295555555556;
      _fe_problem.addUserObject(class_name, "test_fp_liquid", params);
    }
    {
      const std::string class_name = "IdealGasFluidProperties";
      InputParameters params = _app.getFactory().getValidParams(class_name);
      params.set<Real>("gamma") = 1.1;
      params.set<Real>("molar_mass") = 0.027714866;
      params.set<Real>("T_c") = 100;
      params.set<Real>("rho_c") = 300;
      _fe_problem.addUserObject(class_name, "test_fp_vapor", params);
    }
  }

  // create 2-phase fluid properties
  if (_tid == 0)
  {
    const std::string class_name = "TestTwoPhaseFluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);
    params.set<UserObjectName>("fp_liquid") = "test_fp_liquid";
    params.set<UserObjectName>("fp_vapor") = "test_fp_vapor";
    _fe_problem.addUserObject(class_name, _2phase_name, params);
  }
  _fp_2phase = &_fe_problem.getUserObject<TwoPhaseFluidProperties>(_2phase_name);

  // create vapor mixture fluid properties
  if (_tid == 0)
  {
    const std::string class_name = "IdealRealGasMixtureFluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);
    params.set<UserObjectName>("fp_primary") = getVaporName();
    params.set<std::vector<UserObjectName>>("fp_secondary") =
        getParam<std::vector<UserObjectName>>("fp_ncgs");
    _fe_problem.addUserObject(class_name, _vapor_mixture_name, params);
  }
  _fp_vapor_mixture =
      &_fe_problem.getUserObject<VaporMixtureFluidProperties>(_vapor_mixture_name);
}
