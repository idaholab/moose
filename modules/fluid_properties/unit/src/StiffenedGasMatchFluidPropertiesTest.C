//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StiffenedGasMatchFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"
#include "SodiumLiquidFluidProperties.h"
#include "StiffenedGasMatchFluidProperties.h"

StiffenedGasMatchFluidPropertiesTest::StiffenedGasMatchFluidPropertiesTest()
: MooseObjectUnitTest("FluidPropertiesApp"),
  _p(1e5),
  _T(300)
  {
    buildObjects();
  }

void
StiffenedGasMatchFluidPropertiesTest::buildObjects()
{
  const std::string fp_ref_name = "fp_ref";
  const std::string fp_sgfit_name = "fp_sgfit";

  // Reference
  {
    const std::string class_name = "SodiumLiquidFluidProperties";
    InputParameters params = _factory.getValidParams(class_name);
    _fe_problem->addUserObject(class_name, fp_ref_name, params);
    _fp_ref = &_fe_problem->getUserObject<SinglePhaseFluidProperties>(fp_ref_name);
  }

  // SG fit
  {
    const std::string class_name = "StiffenedGasMatchFluidProperties";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("fluid_properties") = {fp_ref_name};
    params.set<Real>("p") = _p;
    params.set<Real>("T") = _T;
    _fe_problem->addUserObject(class_name, fp_sgfit_name, params);
    _fp_sgfit = &_fe_problem->getUserObject<SinglePhaseFluidProperties>(fp_sgfit_name);
  }

  _fp_sgfit->initialSetup();
}


TEST_F(StiffenedGasMatchFluidPropertiesTest, test)
{
  REL_TEST(_fp_sgfit->molarMass(), _fp_ref->molarMass(), REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_sgfit->cp_from_p_T(_p, _T), _fp_ref->cp_from_p_T(_p, _T), REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_sgfit->cv_from_p_T(_p, _T), _fp_ref->cv_from_p_T(_p, _T), REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_sgfit->h_from_p_T(_p, _T), _fp_ref->h_from_p_T(_p, _T), REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_sgfit->rho_from_p_T(_p, _T), _fp_ref->rho_from_p_T(_p, _T), REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_sgfit->s_from_p_T(_p, _T), _fp_ref->s_from_p_T(_p, _T), REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_sgfit->mu_from_p_T(_p, _T), _fp_ref->mu_from_p_T(_p, _T), REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_sgfit->k_from_p_T(_p, _T), _fp_ref->k_from_p_T(_p, _T), REL_TOL_SAVED_VALUE);
}
