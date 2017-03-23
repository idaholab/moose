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
#include "NaClFluidPropertiesTest.h"

#include "FEProblem.h"
#include "AppFactory.h"
#include "GeneratedMesh.h"
#include "NaClFluidProperties.h"

CPPUNIT_TEST_SUITE_REGISTRATION(NaClFluidPropertiesTest);

void
NaClFluidPropertiesTest::registerObjects(Factory & factory)
{
  registerUserObject(NaClFluidProperties);
}

void
NaClFluidPropertiesTest::buildObjects()
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

  InputParameters uo_pars = _factory->getValidParams("NaClFluidProperties");
  _fe_problem->addUserObject("NaClFluidProperties", "fp", uo_pars);
  _fp = &_fe_problem->getUserObject<NaClFluidProperties>("fp");
}

void
NaClFluidPropertiesTest::setUp()
{
  char str[] = "foo";
  char * argv[] = {str, NULL};

  _app = AppFactory::createApp("MooseUnitApp", 1, (char **)argv);
  _factory = &_app->getFactory();

  registerObjects(*_factory);
  buildObjects();
}

void
NaClFluidPropertiesTest::tearDown()
{
  delete _fe_problem;
  delete _mesh;
  delete _app;
}

void
NaClFluidPropertiesTest::halite()
{
  // Density and cp
  Real p0, p1, p2, T0, T1, T2;

  p0 = 30.0e6;
  p1 = 60.0e6;
  p2 = 80.0e6;
  T0 = 300.0;
  T1 = 500.0;
  T2 = 700.0;

  REL_TEST("rho", _fp->rho(p0, T0), 2167.88, 1.0e-3);
  REL_TEST("rho", _fp->rho(p1, T1), 2116.0, 1.0e-3);
  REL_TEST("rho", _fp->rho(p2, T2), 2056.8, 1.0e-3);
  REL_TEST("cp", _fp->cp(p0, T0), 0.865e3, 4.0e-2);
  REL_TEST("cp", _fp->cp(p1, T1), 0.922e3, 4.0e-2);
  REL_TEST("cp", _fp->cp(p2, T2), 0.979e3, 4.0e-2);

  // Test enthalpy at the triple point pressure of water
  Real pt = 611.657;

  ABS_TEST("enthalpy", _fp->h(pt, 273.16), 0.0, 1.0e-3);
  REL_TEST("enthalpy", _fp->h(pt, 573.15), 271.13e3, 1.0e-3);
  REL_TEST("enthalpy", _fp->h(pt, 673.15), 366.55e3, 1.0e-3);

  // Thermal conductivity (function of T only)
  REL_TEST("k", _fp->k(p0, 323.15), 5.488, 1.0e-2);
  REL_TEST("k", _fp->k(p0, 423.15), 3.911, 1.0e-2);
  REL_TEST("k", _fp->k(p0, 523.15), 3.024, 2.0e-2);
}

void
NaClFluidPropertiesTest::derivatives()
{
  Real p = 30.0e6;
  Real T = 300.0;

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
}
