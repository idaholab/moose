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
#include "IdealGasFluidPropertiesTest.h"

#include "FEProblem.h"
#include "AppFactory.h"
#include "GeneratedMesh.h"
#include "IdealGasFluidProperties.h"

CPPUNIT_TEST_SUITE_REGISTRATION(IdealGasFluidPropertiesTest);

void
IdealGasFluidPropertiesTest::registerObjects(Factory & factory)
{
  registerUserObject(IdealGasFluidProperties);
}

void
IdealGasFluidPropertiesTest::buildObjects()
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

  InputParameters uo_pars = _factory->getValidParams("IdealGasFluidProperties");
  uo_pars.set<Real>("R") = 287.04;
  uo_pars.set<Real>("gamma") = 1.41;
  _fe_problem->addUserObject("IdealGasFluidProperties", "fp", uo_pars);
  _fp = &_fe_problem->getUserObject<IdealGasFluidProperties>("fp");
}

void
IdealGasFluidPropertiesTest::setUp()
{
  char str[] = "foo";
  char * argv[] = {str, NULL};

  _app = AppFactory::createApp("MooseUnitApp", 1, (char **)argv);
  _factory = &_app->getFactory();

  registerObjects(*_factory);
  buildObjects();
}

void
IdealGasFluidPropertiesTest::tearDown()
{
  delete _fe_problem;
  delete _mesh;
  delete _app;
}

void
IdealGasFluidPropertiesTest::testAll()
{
  Real T = 120. + 273.15; // K
  Real p = 101325;        // Pa

  Real rho = _fp->rho(p, T);
  Real v = 1 / rho;
  Real e;

  REL_TEST("rho", _fp->rho(p, T), 0.897875065343506, 1e-15);
  REL_TEST("e", _fp->e(p, rho), 275243.356097561, 1e-15);

  _fp->rho_e(p, T, rho, e);
  REL_TEST("rho", rho, 0.897875065343506, 1e-15);
  REL_TEST("e", e, 275243.356097561, 1e-15);

  REL_TEST("p", _fp->pressure(v, e), p, 1e-15);
  REL_TEST("T", _fp->temperature(v, e), T, 1e-15);
  REL_TEST("c", _fp->c(v, e), 398.896207251962, 1e-15);
  REL_TEST("cp", _fp->cp(v, e), 987.13756097561, 1e-15);
  REL_TEST("cv", _fp->cv(v, e), 700.09756097561, 1e-15);

  REL_TEST("h", _fp->h(p, T), 388093, 4e-7);

  // derivatives
  p = 1e6;
  T = 500;

  Real de = 1e-3;
  Real dv = 1e-6;
  {
    Real dp_dv_fd = (_fp->pressure(v + dv, e) - _fp->pressure(v - dv, e)) / (2 * dv);
    Real dp_de_fd = (_fp->pressure(v, e + de) - _fp->pressure(v, e - de)) / (2 * de);

    Real dT_dv_fd = (_fp->temperature(v + dv, e) - _fp->temperature(v - dv, e)) / (2 * dv);
    Real dT_de_fd = (_fp->temperature(v, e + de) - _fp->temperature(v, e - de)) / (2 * de);

    Real dp_dv, dp_de, dT_dv, dT_de;
    _fp->dp_duv(v, e, dp_dv, dp_de, dT_dv, dT_de);

    ABS_TEST("dp_dv", dp_dv, dp_dv_fd, 4e-6);
    ABS_TEST("dp_de", dp_de, dp_de_fd, 1e-8);
    ABS_TEST("dT_dv", dT_dv, dT_dv_fd, 1e-15);
    ABS_TEST("dT_de", dT_de, dT_de_fd, 1e-11);
  }

  Real dp = 1e1;
  Real dT = 1e-4;

  {
    // density
    Real drho_dp_fd = (_fp->rho(p + dp, T) - _fp->rho(p - dp, T)) / (2 * dp);
    Real drho_dT_fd = (_fp->rho(p, T + dT) - _fp->rho(p, T - dT)) / (2 * dT);
    Real rho = 0, drho_dp = 0, drho_dT = 0;
    _fp->rho_dpT(p, T, rho, drho_dp, drho_dT);

    ABS_TEST("rho", rho, _fp->rho(p, T), 1e-15);
    ABS_TEST("drho_dp", drho_dp, drho_dp_fd, 1e-15);
    ABS_TEST("drho_dT", drho_dT, drho_dT_fd, 1e-11);
  }

  rho = _fp->rho(p, T);
  Real drho = 1e-4;

  {
    // internal energy
    Real de_dp_fd = (_fp->e(p + dp, rho) - _fp->e(p - dp, rho)) / (2 * dp);
    Real de_drho_fd = (_fp->e(p, rho + drho) - _fp->e(p, rho - drho)) / (2 * drho);
    Real de_dp = 0, de_drho = 0;
    _fp->e_dprho(p, rho, e, de_dp, de_drho);

    ABS_TEST("e", e, _fp->e(p, rho), 1e-16);
    ABS_TEST("de_dp", de_dp, de_dp_fd, 5e-11);
    ABS_TEST("de_drho", de_drho, de_drho_fd, 5e-5);
  }

  {
    Real de_dp, de_dT;
    _fp->e_dpT(p, T, e, de_dp, de_dT);

    Real rho1, rho2;
    Real e1, e2;

    _fp->rho_e(p + dp, T, rho1, e1);
    _fp->rho_e(p - dp, T, rho2, e2);
    Real de_dp_fd = (e1 - e2) / (2 * dp);

    _fp->rho_e(p, T + dT, rho1, e1);
    _fp->rho_e(p, T - dT, rho2, e2);
    Real de_dT_fd = (e1 - e2) / (2 * dT);

    ABS_TEST("de_dp", de_dp, de_dp_fd, 1e-15);
    ABS_TEST("de_dT", de_dT, de_dT_fd, 5e-7);
  }

  {
    // enthalpy
    Real dh_dp_fd = (_fp->h(p + dp, T) - _fp->h(p - dp, T)) / (2 * dp);
    Real dh_dT_fd = (_fp->h(p, T + dT) - _fp->h(p, T - dT)) / (2 * dT);

    Real h = 0, dh_dp = 0, dh_dT = 0;
    _fp->h_dpT(p, T, h, dh_dp, dh_dT);

    ABS_TEST("h", h, _fp->h(p, T), 1e-16);
    ABS_TEST("dh_dp", dh_dp, dh_dp_fd, 1e-12);
    ABS_TEST("dh_dT", dh_dT, dh_dT_fd, 6e-7);
  }
}
