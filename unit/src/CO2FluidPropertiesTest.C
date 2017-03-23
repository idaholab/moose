/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MooseApp.h"
#include "Utils.h"
#include "CO2FluidPropertiesTest.h"

#include "FEProblem.h"
#include "AppFactory.h"
#include "GeneratedMesh.h"
#include "CO2FluidProperties.h"

CPPUNIT_TEST_SUITE_REGISTRATION(CO2FluidPropertiesTest);

void
CO2FluidPropertiesTest::registerObjects(Factory & factory)
{
  registerUserObject(CO2FluidProperties);
}

void
CO2FluidPropertiesTest::buildObjects()
{
  InputParameters mesh_params = _factory->getValidParams("GeneratedMesh");
  mesh_params.set<MooseEnum>("dim") = "3";
  mesh_params.set<std::string>("name") = "mesh";
  mesh_params.set<std::string>("_object_name") = "name1";
  _mesh = new GeneratedMesh(mesh_params);

  InputParameters problem_params = _factory->getValidParams("FEProblem");
  problem_params.set<MooseMesh *>("mesh") = _mesh;
  problem_params.set<std::string>("name") = "problem";
  problem_params.set<std::string>("_object_name") = "name2";
  _fe_problem = new FEProblem(problem_params);

  InputParameters uo_pars = _factory->getValidParams("CO2FluidProperties");
  _fe_problem->addUserObject("CO2FluidProperties", "fp", uo_pars);
  _fp = &_fe_problem->getUserObject<CO2FluidProperties>("fp");
}

void
CO2FluidPropertiesTest::setUp()
{
  char str[] = "foo";
  char * argv[] = {str, NULL};

  _app = AppFactory::createApp("MooseUnitApp", 1, (char **)argv);
  _factory = &_app->getFactory();

  registerObjects(*_factory);
  buildObjects();
}

void
CO2FluidPropertiesTest::tearDown()
{
  delete _fe_problem;
  delete _mesh;
  delete _app;
}

void
CO2FluidPropertiesTest::melting()
{
  REL_TEST("melting", _fp->meltingPressure(217.03), 2.57e6, 1.0e-2);
  REL_TEST("melting", _fp->meltingPressure(235.29), 95.86e6, 1.0e-2);
  REL_TEST("melting", _fp->meltingPressure(266.04), 286.77e6, 1.0e-2);
}

void
CO2FluidPropertiesTest::sublimation()
{
  REL_TEST("sublimation", _fp->sublimationPressure(194.6857), 0.101325e6, 1.0e-4);
}

void
CO2FluidPropertiesTest::vapor()
{
  // Vapor pressure
  REL_TEST("vapor", _fp->vaporPressure(217.0), 0.52747e6, 1.0e-2);
  REL_TEST("vapor", _fp->vaporPressure(245.0), 1.51887e6, 1.0e-2);
  REL_TEST("vapor", _fp->vaporPressure(303.8), 7.32029e6, 1.0e-2);

  // Saturated vapor density
  REL_TEST("saturated vapor density", _fp->saturatedVaporDensity(217.0), 14.0017, 1.0e-2);
  REL_TEST("saturated vapor density", _fp->saturatedVaporDensity(245.0), 39.5048, 1.0e-2);
  REL_TEST("saturated vapor density", _fp->saturatedVaporDensity(303.8), 382.30, 1.0e-2);

  // Saturated liquid density
  REL_TEST("saturated liquid density", _fp->saturatedLiquidDensity(217.0), 1177.03, 1.0e-2);
  REL_TEST("saturated liquid density", _fp->saturatedLiquidDensity(245.0), 1067.89, 1.0e-2);
  REL_TEST("saturated liquid density", _fp->saturatedLiquidDensity(303.8), 554.14, 1.0e-2);
}

void
CO2FluidPropertiesTest::partialDensity()
{
  REL_TEST("partial density", _fp->partialDensity(373.15), 1182.8, 5.0e-2);
  REL_TEST("partial density", _fp->partialDensity(473.35), 880.0, 5.0e-2);
  REL_TEST("partial density", _fp->partialDensity(573.15), 593.8, 5.0e-2);
}

void
CO2FluidPropertiesTest::henry()
{
  REL_TEST("henry", _fp->henryConstant(300.0), 173.63e6, 1.0e-3);
  REL_TEST("henry", _fp->henryConstant(400.0), 579.84e6, 1.0e-3);
  REL_TEST("henry", _fp->henryConstant(500.0), 520.79e6, 1.0e-3);
  REL_TEST("henry", _fp->henryConstant(600.0), 259.53e6, 1.0e-3);
}

void
CO2FluidPropertiesTest::thermalConductivity()
{
  REL_TEST("thermal conductivity", _fp->k(23.435, 250.0), 13.45e-3, 1.0e-3);
  REL_TEST("thermal conductivity", _fp->k(18.579, 300.0), 17.248e-3, 1.0e-3);
  REL_TEST("thermal conductivity", _fp->k(11.899, 450.0), 29.377e-3, 1.0e-3);
}

void
CO2FluidPropertiesTest::viscosity()
{
  REL_TEST("viscosity", _fp->mu(20.199, 280.0), 14.15e-6, 1.0e-3);
  REL_TEST("viscosity", _fp->mu(15.105, 360.0), 17.94e-6, 1.0e-3);
  REL_TEST("viscosity", _fp->mu(10.664, 500.0), 24.06e-6, 1.0e-3);
}

void
CO2FluidPropertiesTest::propertiesSW()
{
  // Pressure = 1 MPa, temperature = 280 K
  Real p = 1.0e6;
  Real T = 280.0;
  REL_TEST("density", _fp->rho(p, T), 20.199, 1.0e-3);
  REL_TEST("enthalpy", _fp->h(p, T), -26.385e3, 1.0e-3);
  REL_TEST("internal energy", _fp->e(p, T), -75.892e3, 1.0e-3);
  REL_TEST("entropy", _fp->s(p, T), -0.51326e3, 1.0e-3);
  REL_TEST("cp", _fp->cp(p, T), 0.92518e3, 1.0e-3);
  REL_TEST("cv", _fp->cv(p, T), 0.67092e3, 1.0e-3);
  REL_TEST("c", _fp->c(p, T), 252.33, 1.0e-3);

  // Pressure = 1 MPa, temperature = 500 K
  T = 500.0;
  REL_TEST("density", _fp->rho(p, T), 10.664, 1.0e-3);
  REL_TEST("enthalpy", _fp->h(p, T), 185.60e3, 1.0e-3);
  REL_TEST("internal energy", _fp->e(p, T), 91.829e3, 1.0e-3);
  REL_TEST("entropy", _fp->s(p, T), 0.04225e3, 1.0e-3);
  REL_TEST("cp", _fp->cp(p, T), 1.0273e3, 1.0e-3);
  REL_TEST("cv", _fp->cv(p, T), 0.82823e3, 1.0e-3);
  REL_TEST("c", _fp->c(p, T), 339.81, 1.0e-3);

  // Pressure = 10 MPa, temperature = 500 K
  p = 10.0e6;
  REL_TEST("density", _fp->rho(p, T), 113.07, 1.0e-3);
  REL_TEST("enthalpy", _fp->h(p, T), 157.01e3, 1.0e-3);
  REL_TEST("internal energy", _fp->e(p, T), 68.569e3, 1.0e-3);
  REL_TEST("entropy", _fp->s(p, T), -0.4383e3, 1.0e-3);
  REL_TEST("cp", _fp->cp(p, T), 1.1624e3, 1.0e-3);
  REL_TEST("cv", _fp->cv(p, T), 0.85516e3, 1.0e-3);
  REL_TEST("c", _fp->c(p, T), 337.45, 1.0e-3);
}

void
CO2FluidPropertiesTest::derivatives()
{
  Real p = 1.0e6;
  Real T = 350.0;

  // Finite differencing parameters
  Real dp = 1.0e1;
  Real dT = 1.0e-4;

  // density
  Real drho_dp_fd = (_fp->rho(p + dp, T) - _fp->rho(p - dp, T)) / (2.0 * dp);
  Real drho_dT_fd = (_fp->rho(p, T + dT) - _fp->rho(p, T - dT)) / (2.0 * dT);
  Real rho = 0.0, drho_dp = 0.0, drho_dT = 0.0;
  _fp->rho_dpT(p, T, rho, drho_dp, drho_dT);

  ABS_TEST("rho", rho, _fp->rho(p, T), 1.0e-15);
  REL_TEST("drho_dp", drho_dp, drho_dp_fd, 1.0e-6);
  REL_TEST("drho_dT", drho_dT, drho_dT_fd, 1.0e-6);

  // enthalpy
  Real dh_dp_fd = (_fp->h(p + dp, T) - _fp->h(p - dp, T)) / (2.0 * dp);
  Real dh_dT_fd = (_fp->h(p, T + dT) - _fp->h(p, T - dT)) / (2.0 * dT);
  Real h = 0.0, dh_dp = 0.0, dh_dT = 0.0;
  _fp->h_dpT(p, T, h, dh_dp, dh_dT);

  ABS_TEST("h", h, _fp->h(p, T), 1.0e-15);
  REL_TEST("dh_dp", dh_dp, dh_dp_fd, 1.0e-6);
  REL_TEST("dh_dT", dh_dT, dh_dT_fd, 1.0e-6);

  // internal energy
  Real de_dp_fd = (_fp->e(p + dp, T) - _fp->e(p - dp, T)) / (2.0 * dp);
  Real de_dT_fd = (_fp->e(p, T + dT) - _fp->e(p, T - dT)) / (2.0 * dT);
  Real e = 0.0, de_dp = 0.0, de_dT = 0.0;
  _fp->e_dpT(p, T, e, de_dp, de_dT);

  ABS_TEST("e", e, _fp->e(p, T), 1.0e-15);
  REL_TEST("de_dp", de_dp, de_dp_fd, 1.0e-6);
  REL_TEST("de_dT", de_dT, de_dT_fd, 1.0e-6);

  // Viscosity
  rho = 15.105;
  T = 360.0;
  Real drho = 1.0e-4;

  Real dmu_drho_fd = (_fp->mu(rho + drho, T) - _fp->mu(rho - drho, T)) / (2.0 * drho);
  Real dmu_dT_fd = (_fp->mu(rho, T + dT) - _fp->mu(rho, T - dT)) / (2.0 * dT);
  Real mu = 0.0, dmu_drho = 0.0, dmu_dT = 0.0;
  _fp->mu_drhoT(rho, T, mu, dmu_drho, dmu_dT);

  ABS_TEST("mu", mu, _fp->mu(rho, T), 1.0e-15);
  REL_TEST("dmu_dp", dmu_drho, dmu_drho_fd, 1.0e-6);
  REL_TEST("dmu_dT", dmu_dT, dmu_dT_fd, 1.0e-6);

  // Henry's constant
  T = 300.0;

  Real dKh_dT_fd = (_fp->henryConstant(T + dT) - _fp->henryConstant(T - dT)) / (2.0 * dT);
  Real Kh = 0.0, dKh_dT = 0.0;
  _fp->henryConstant_dT(T, Kh, dKh_dT);
  REL_TEST("henry", Kh, _fp->henryConstant(T), 1.0e-6);
  REL_TEST("dhenry_dT", dKh_dT_fd, dKh_dT, 1.0e-6);
}
