//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestNumericalFlux3EqnBase.h"
#include "FlowModel1PhaseUtils.h"
#include "IdealGasFluidProperties.h"

TestNumericalFlux3EqnBase::TestNumericalFlux3EqnBase()
  : TestNumericalFlux1D(),
    _fp_name("fp"),
    _fp(getFluidPropertiesObject())
{
}

std::vector<ADReal>
TestNumericalFlux3EqnBase::computeConservativeSolution(const std::vector<ADReal> & W,
                                                       const ADReal & A) const
{
  return FlowModel1PhaseUtils::computeConservativeSolutionVector<true>(W, A, _fp);
}

std::vector<ADReal>
TestNumericalFlux3EqnBase::computeFluxFromPrimitive(const std::vector<ADReal> & W,
                                                    const ADReal & A) const
{
  return FlowModel1PhaseUtils::computeFluxFromPrimitive<true>(W, A, _fp);
}

const SinglePhaseFluidProperties &
TestNumericalFlux3EqnBase::getFluidPropertiesObject()
{
  const std::string class_name = "IdealGasFluidProperties";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<Real>("gamma") = 1.4;
  params.set<Real>("molar_mass") = 11.640243719999999;
  _fe_problem->addUserObject(class_name, _fp_name, params);

  return _fe_problem->getUserObject<SinglePhaseFluidProperties>(_fp_name);
}
