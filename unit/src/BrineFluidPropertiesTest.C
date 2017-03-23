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
#include "BrineFluidPropertiesTest.h"

#include "FEProblem.h"
#include "AppFactory.h"
#include "GeneratedMesh.h"
#include "BrineFluidProperties.h"
#include "Water97FluidProperties.h"
#include "NaClFluidProperties.h"

CPPUNIT_TEST_SUITE_REGISTRATION(BrineFluidPropertiesTest);

void
BrineFluidPropertiesTest::registerObjects(Factory & factory)
{
  registerUserObject(BrineFluidProperties);
  registerUserObject(Water97FluidProperties);
  registerUserObject(NaClFluidProperties);
}

void
BrineFluidPropertiesTest::buildObjects()
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

  // The brine fluid properties
  InputParameters uo_pars = _factory->getValidParams("BrineFluidProperties");
  _fe_problem->addUserObject("BrineFluidProperties", "fp", uo_pars);
  _fp = &_fe_problem->getUserObject<BrineFluidProperties>("fp");

  // Get the water properties UserObject
  _water_fp = &_fp->getComponent(BrineFluidProperties::WATER);
}

void
BrineFluidPropertiesTest::setUp()
{
  char str[] = "foo";
  char * argv[] = {str, NULL};

  _app = AppFactory::createApp("MooseUnitApp", 1, (char **)argv);
  _factory = &_app->getFactory();

  registerObjects(*_factory);
  buildObjects();
}

void
BrineFluidPropertiesTest::tearDown()
{
  delete _fe_problem;
  delete _mesh;
  delete _app;
}

void
BrineFluidPropertiesTest::vapor()
{
  REL_TEST("vapor", _fp->pSat(473.15, 0.185), 1.34e6, 1.0e-2);
  REL_TEST("vapor", _fp->pSat(473.15, 0.267), 1.21e6, 1.0e-2);
  REL_TEST("vapor", _fp->pSat(473.15, 0.312), 1.13e6, 1.0e-2);
}

void
BrineFluidPropertiesTest::solubility()
{
  REL_TEST("halite solubility", _fp->haliteSolubility(659.65), 0.442, 2.0e-2);
  REL_TEST("halite solubility", _fp->haliteSolubility(818.65), 0.6085, 2.0e-2);
  REL_TEST("halite solubility", _fp->haliteSolubility(903.15), 0.7185, 2.0e-2);
}

void
BrineFluidPropertiesTest::properties()
{
  // Pressure, temperature and NaCl mass fraction for tests
  Real p0 = 20.0e6;
  Real p1 = 40.0e6;
  Real T0 = 323.15;
  Real T1 = 473.15;
  Real x0 = 0.1047;
  Real x1 = 0.2261;

  // Density
  REL_TEST("density", _fp->rho(p0, T0, x0), 1068.52, 1.0e-2);
  REL_TEST("density", _fp->rho(p0, T1, x0), 959.27, 1.0e-2);
  REL_TEST("density", _fp->rho(p1, T1, x1), 1065.58, 1.0e-2);

  // Viscosity
  REL_TEST("viscosity", _fp->mu(_water_fp->rho(p0, T0), T0, x0), 679.8e-6, 2.0e-2);
  REL_TEST("viscosity", _fp->mu(_water_fp->rho(p0, T1), T1, x0), 180.0e-6, 2.0e-2);
  REL_TEST("viscosity", _fp->mu(_water_fp->rho(p1, T1), T1, x1), 263.1e-6, 2.0e-2);

  // Thermal conductivity
  REL_TEST("thermal conductivity", _fp->k(_water_fp->rho(p0, T0), T0, x0), 0.630, 4.0e-2);
  REL_TEST("thermal conductivity", _fp->k(_water_fp->rho(p0, T1), T1, x0), 0.649, 4.0e-2);
  REL_TEST("thermal conductivity", _fp->k(_water_fp->rho(p1, T1), T1, x1), 0.633, 4.0e-2);

  // Enthalpy
  p0 = 10.0e6;
  T0 = 573.15;

  REL_TEST("enthalpy", _fp->e(p0, T0, 0.0), 1330.0e3, 1.0e-2);
  REL_TEST("enthalpy", _fp->e(p0, T0, 0.2), 1100.0e3, 1.0e-2);
  REL_TEST("enthalpy", _fp->e(p0, T0, 0.364), 970.0e3, 1.0e-2);

  // cp
  p0 = 17.9e6;
  x0 = 0.01226;

  REL_TEST("cp", _fp->cp(p0, 323.15, x0), 4.1e3, 1.0e-2);
  REL_TEST("cp", _fp->cp(p0, 473.15, x0), 4.35e3, 1.0e-2);
  REL_TEST("cp", _fp->cp(p0, 623.15, x0), 8.1e3, 1.0e-2);
}

void
BrineFluidPropertiesTest::derivatives()
{
  Real p = 1.0e6;
  Real T = 350.0;
  Real x = 0.1047;

  // Finite differencing parameters
  Real dp = 1.0e-2;
  Real dT = 1.0e-4;
  Real dx = 1.0e-6;

  // density
  Real drho_dp_fd = (_fp->rho(p + dp, T, x) - _fp->rho(p - dp, T, x)) / (2.0 * dp);
  Real drho_dT_fd = (_fp->rho(p, T + dT, x) - _fp->rho(p, T - dT, x)) / (2.0 * dT);
  Real drho_dx_fd = (_fp->rho(p, T, x + dx) - _fp->rho(p, T, x - dx)) / (2.0 * dx);

  Real rho = 0.0, drho_dp = 0.0, drho_dT = 0.0, drho_dx = 0.0;
  _fp->rho_dpTx(p, T, x, rho, drho_dp, drho_dT, drho_dx);

  ABS_TEST("rho", rho, _fp->rho(p, T, x), 1.0e-15);
  REL_TEST("drho_dp", drho_dp, drho_dp_fd, 1.0e-3);
  REL_TEST("drho_dT", drho_dT, drho_dT_fd, 1.0e-3);
  REL_TEST("drho_dx", drho_dx, drho_dx_fd, 1.0e-3);

  // enthalpy
  Real dh_dp_fd = (_fp->h(p + dp, T, x) - _fp->h(p - dp, T, x)) / (2.0 * dp);
  Real dh_dT_fd = (_fp->h(p, T + dT, x) - _fp->h(p, T - dT, x)) / (2.0 * dT);
  Real dh_dx_fd = (_fp->h(p, T, x + dx) - _fp->h(p, T, x - dx)) / (2.0 * dx);

  Real h = 0.0, dh_dp = 0.0, dh_dT = 0.0, dh_dx = 0.0;
  _fp->h_dpTx(p, T, x, h, dh_dp, dh_dT, dh_dx);

  ABS_TEST("h", h, _fp->h(p, T, x), 1.0e-15);
  REL_TEST("dh_dp", dh_dp, dh_dp_fd, 1.0e-3);
  REL_TEST("dh_dT", dh_dT, dh_dT_fd, 1.0e-3);
  REL_TEST("dh_dx", dh_dx, dh_dx_fd, 1.0e-3);

  // internal energy
  Real de_dp_fd = (_fp->e(p + dp, T, x) - _fp->e(p - dp, T, x)) / (2.0 * dp);
  Real de_dT_fd = (_fp->e(p, T + dT, x) - _fp->e(p, T - dT, x)) / (2.0 * dT);
  Real de_dx_fd = (_fp->e(p, T, x + dx) - _fp->e(p, T, x - dx)) / (2.0 * dx);

  Real e = 0.0, de_dp = 0.0, de_dT = 0.0, de_dx = 0.0;
  _fp->e_dpTx(p, T, x, e, de_dp, de_dT, de_dx);

  ABS_TEST("e", e, _fp->e(p, T, x), 1.0e-15);
  REL_TEST("de_dp", de_dp, de_dp_fd, 1.0e-1);
  REL_TEST("de_dT", de_dT, de_dT_fd, 1.0e-3);
  REL_TEST("de_dx", de_dx, de_dx_fd, 1.0e-3);

  // Viscosity
  Real drho = 1.0e-4;

  Real dmu_drho_fd = (_fp->mu(rho + drho, T, x) - _fp->mu(rho - drho, T, x)) / (2.0 * drho);
  Real dmu_dT_fd = (_fp->mu(rho, T + dT, x) - _fp->mu(rho, T - dT, x)) / (2.0 * dT);
  Real dmu_dx_fd = (_fp->mu(rho, T, x + dx) - _fp->mu(rho, T, x - dx)) / (2.0 * dx);
  Real mu = 0.0, dmu_drho = 0.0, dmu_dT = 0.0, dmu_dx = 0.0;
  _fp->mu_drhoTx(rho, T, x, mu, dmu_drho, dmu_dT, dmu_dx);

  ABS_TEST("mu", mu, _fp->mu(rho, T, x), 1.0e-15);
  REL_TEST("dmu_dp", dmu_drho, dmu_drho_fd, 1.0e-3);
  REL_TEST("dmu_dT", dmu_dT, dmu_dT_fd, 1.0e-3);
  REL_TEST("dmu_dx", dmu_dx, dmu_dx_fd, 1.0e-3);
}
