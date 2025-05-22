//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestNumericalFluxGasMixBase.h"
#include "IdealGasMixtureFluidProperties.h"
#include "FlowModelGasMixUtils.h"

TestNumericalFluxGasMixBase::TestNumericalFluxGasMixBase()
 : TestNumericalFlux1D(),
  _fp_mix_name("fp_mix")
{
  addFluidProperties();
}

std::vector<ADReal>
TestNumericalFluxGasMixBase::computeConservativeSolution(const std::vector<ADReal> & W,
                                                       const ADReal & A) const
{
  return FlowModelGasMixUtils::computeConservativeSolution<true>(W, A, *_fp_mix);
}

std::vector<ADReal>
TestNumericalFluxGasMixBase::computeFluxFromPrimitive(const std::vector<ADReal> & W,
                                                    const ADReal & A) const
{
  return FlowModelGasMixUtils::computeFluxFromPrimitive<true>(W, A, *_fp_mix);
}

void
TestNumericalFluxGasMixBase::addFluidProperties()
{
  const std::string fp_steam_name = "fp_steam";
  const std::string fp_nitrogen_name = "fp_nitrogen";

  // steam fluid properties; parameters correspond to T in range 298 K to 473 K
  {
    const std::string class_name = "IdealGasFluidProperties";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("gamma") = 1.43;
    params.set<Real>("molar_mass") = 0.01801488;
    params.set<Real>("mu") = 0.000013277592; // at 400 K and 1.e5 Pa
    params.set<Real>("k") = 0.026824977826;  // at 400 K and 1.e5 Pa
    _fe_problem->addUserObject(class_name, fp_steam_name, params);
  }

  // nitrogen fluid properties
  {
    const std::string class_name = "IdealGasFluidProperties";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("gamma") = 1.4;
    params.set<Real>("molar_mass") = 0.028012734746133888;
    params.set<Real>("mu") = 0.0000222084; // at 400 K and 1.e5 Pa
    params.set<Real>("k") = 0.032806168;   // at 400 K and 1.e5 Pa
    _fe_problem->addUserObject(class_name, fp_nitrogen_name, params);
  }

  // mixture fluid properties
  {
    const std::string class_name = "IdealGasMixtureFluidProperties";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<UserObjectName>>("component_fluid_properties") = {fp_steam_name, fp_nitrogen_name};
    _fe_problem->addUserObject(class_name, _fp_mix_name, params);
    _fp_mix = &_fe_problem->getUserObject<IdealGasMixtureFluidProperties>(_fp_mix_name);
  }
}
