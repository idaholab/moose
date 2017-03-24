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
#include "MethaneFluidPropertiesTest.h"

#include "FEProblem.h"
#include "AppFactory.h"
#include "GeneratedMesh.h"
#include "MethaneFluidProperties.h"

CPPUNIT_TEST_SUITE_REGISTRATION(MethaneFluidPropertiesTest);

void
MethaneFluidPropertiesTest::registerObjects(Factory & factory)
{
  registerUserObject(MethaneFluidProperties);
}

void
MethaneFluidPropertiesTest::buildObjects()
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

  InputParameters uo_pars = _factory->getValidParams("MethaneFluidProperties");
  _fe_problem->addUserObject("MethaneFluidProperties", "fp", uo_pars);
  _fp = &_fe_problem->getUserObject<MethaneFluidProperties>("fp");
}

void
MethaneFluidPropertiesTest::setUp()
{
  char str[] = "foo";
  char * argv[] = {str, NULL};

  _app = AppFactory::createApp("MooseUnitApp", 1, (char **)argv);
  _factory = &_app->getFactory();

  registerObjects(*_factory);
  buildObjects();
}

void
MethaneFluidPropertiesTest::tearDown()
{
  delete _fe_problem;
  delete _mesh;
  delete _app;
}

void
MethaneFluidPropertiesTest::henry()
{
  REL_TEST("henry", _fp->henryConstant(300.0), 4069.0e6, 1.0e-3);
  REL_TEST("henry", _fp->henryConstant(400.0), 6017.1e6, 1.0e-3);
  REL_TEST("henry", _fp->henryConstant(500.0), 2812.9e6, 1.0e-3);
  REL_TEST("henry", _fp->henryConstant(600.0), 801.8e6, 1.0e-3);
}

void
MethaneFluidPropertiesTest::properties()
{
  // Pressure = 10 MPa, temperature = 350 K
  Real p = 10.0e6;
  Real T = 350.0;

  REL_TEST("density", _fp->rho(p, T), 55.13, 1.0e-3);
  REL_TEST("enthalpy", _fp->h(p, T), 708.5e3, 1.0e-3);
  REL_TEST("internal energy", _fp->e(p, T), 527.131e3, 1.0e-3);
  REL_TEST("entropy", _fp->s(p, T), 11.30e3, 1.0e-3);
  REL_TEST("cp", _fp->cp(p, T), 2.375e3, 1.0e-3);
  REL_TEST("cv", _fp->cv(p, T), 1.857e3, 1.0e-3);
  REL_TEST("c", _fp->c(p, T), 481.7, 1.0e-3);
  REL_TEST("mu", _fp->mu(0.0, T), 0.01276e-3, 1.0e-3);
  REL_TEST("thermal conductivity", _fp->k(0.0, T), 0.04113, 1.0e-3);
}

void
MethaneFluidPropertiesTest::derivatives()
{
  Real p = 10.0e6;
  Real T = 350.0;

  // Finite differencing parameters
  Real dp = 1.0e1;
  Real dT = 1.0e-4;

  // Density
  Real drho_dp_fd = (_fp->rho(p + dp, T) - _fp->rho(p - dp, T)) / (2.0 * dp);
  Real drho_dT_fd = (_fp->rho(p, T + dT) - _fp->rho(p, T - dT)) / (2.0 * dT);
  Real rho = 0.0, drho_dp = 0.0, drho_dT = 0.0;
  _fp->rho_dpT(p, T, rho, drho_dp, drho_dT);

  ABS_TEST("rho", rho, _fp->rho(p, T), 1.0e-15);
  REL_TEST("drho_dp", drho_dp, drho_dp_fd, 1.0e-6);
  REL_TEST("drho_dT", drho_dT, drho_dT_fd, 1.0e-6);

  // Enthalpy
  Real dh_dp_fd = (_fp->h(p + dp, T) - _fp->h(p - dp, T)) / (2.0 * dp);
  Real dh_dT_fd = (_fp->h(p, T + dT) - _fp->h(p, T - dT)) / (2.0 * dT);
  Real h = 0.0, dh_dp = 0.0, dh_dT = 0.0;
  _fp->h_dpT(p, T, h, dh_dp, dh_dT);

  ABS_TEST("h", h, _fp->h(p, T), 1.0e-15);
  ABS_TEST("dh_dp", dh_dp, dh_dp_fd, 1.0e-15);
  REL_TEST("dh_dT", dh_dT, dh_dT_fd, 1.0e-6);

  // Internal energy
  Real de_dp_fd = (_fp->e(p + dp, T) - _fp->e(p - dp, T)) / (2.0 * dp);
  Real de_dT_fd = (_fp->e(p, T + dT) - _fp->e(p, T - dT)) / (2.0 * dT);
  Real e = 0.0, de_dp = 0.0, de_dT = 0.0;
  _fp->e_dpT(p, T, e, de_dp, de_dT);

  ABS_TEST("e", e, _fp->e(p, T), 1.0e-15);
  ABS_TEST("de_dp", de_dp, de_dp_fd, 1.0e-15);
  REL_TEST("de_dT", de_dT, de_dT_fd, 1.0e-6);

  // Viscosity
  rho = 1.0; // Not used in correlations
  Real drho = 1.0e-4;

  Real dmu_drho_fd = (_fp->mu(rho + drho, T) - _fp->mu(rho - drho, T)) / (2.0 * drho);
  Real dmu_dT_fd = (_fp->mu(rho, T + dT) - _fp->mu(rho, T - dT)) / (2.0 * dT);
  Real mu = 0.0, dmu_drho = 0.0, dmu_dT = 0.0;
  _fp->mu_drhoT(rho, T, mu, dmu_drho, dmu_dT);

  ABS_TEST("mu", mu, _fp->mu(rho, T), 1.0e-15);
  ABS_TEST("dmu_dp", dmu_drho, dmu_drho_fd, 1.0e-15);
  REL_TEST("dmu_dT", dmu_dT, dmu_dT_fd, 1.0e-6);

  // Henry's constant
  T = 300.0;

  Real dKh_dT_fd = (_fp->henryConstant(T + dT) - _fp->henryConstant(T - dT)) / (2.0 * dT);
  Real Kh = 0.0, dKh_dT = 0.0;
  _fp->henryConstant_dT(T, Kh, dKh_dT);
  REL_TEST("henry", Kh, _fp->henryConstant(T), 1.0e-6);
  REL_TEST("dhenry_dT", dKh_dT_fd, dKh_dT, 1.0e-6);
}
