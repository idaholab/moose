//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef WATER97FLUIDPROPERTIESTEST_H
#define WATER97FLUIDPROPERTIESTEST_H

// CPPUnit includes
#include "gtest/gtest.h"

#include "MooseApp.h"
#include "Utils.h"
#include "FEProblem.h"
#include "AppFactory.h"
#include "GeneratedMesh.h"
#include "Water97FluidProperties.h"

class Water97FluidPropertiesTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    const char * argv[] = {"foo", NULL};

    _app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
    _factory = &_app->getFactory();
    registerObjects(*_factory);
    buildObjects();
  }

  void registerObjects(Factory & factory) { registerUserObject(Water97FluidProperties); }

  void buildObjects()
  {
    InputParameters mesh_params = _factory->getValidParams("GeneratedMesh");
    mesh_params.set<MooseEnum>("dim") = "3";
    mesh_params.set<std::string>("name") = "mesh";
    mesh_params.set<std::string>("_object_name") = "name1";
    _mesh = libmesh_make_unique<GeneratedMesh>(mesh_params);

    InputParameters problem_params = _factory->getValidParams("FEProblem");
    problem_params.set<MooseMesh *>("mesh") = _mesh.get();
    problem_params.set<std::string>("name") = "problem";
    problem_params.set<std::string>("_object_name") = "name2";
    _fe_problem = libmesh_make_unique<FEProblem>(problem_params);

    InputParameters uo_pars = _factory->getValidParams("Water97FluidProperties");
    _fe_problem->addUserObject("Water97FluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<Water97FluidProperties>("fp");
  }

  void regionDerivatives(Real p, Real T, Real tol)
  {
    // Finite differencing parameters
    Real dp = 1.0e1;
    Real dT = 1.0e-4;

    // density
    Real drho_dp_fd = (_fp->rho(p + dp, T) - _fp->rho(p - dp, T)) / (2.0 * dp);
    Real drho_dT_fd = (_fp->rho(p, T + dT) - _fp->rho(p, T - dT)) / (2.0 * dT);
    Real rho = 0.0, drho_dp = 0.0, drho_dT = 0.0;
    _fp->rho_dpT(p, T, rho, drho_dp, drho_dT);

    ABS_TEST("rho", rho, _fp->rho(p, T), 1.0e-15);
    REL_TEST("drho_dp", drho_dp, drho_dp_fd, tol);
    REL_TEST("drho_dT", drho_dT, drho_dT_fd, tol);

    // enthalpy
    Real dh_dp_fd = (_fp->h(p + dp, T) - _fp->h(p - dp, T)) / (2.0 * dp);
    Real dh_dT_fd = (_fp->h(p, T + dT) - _fp->h(p, T - dT)) / (2.0 * dT);
    Real h = 0.0, dh_dp = 0.0, dh_dT = 0.0;
    _fp->h_dpT(p, T, h, dh_dp, dh_dT);

    ABS_TEST("h", h, _fp->h(p, T), 1.0e-15);
    REL_TEST("dh_dp", dh_dp, dh_dp_fd, tol);
    REL_TEST("dh_dT", dh_dT, dh_dT_fd, tol);

    // internal energy
    Real de_dp_fd = (_fp->e(p + dp, T) - _fp->e(p - dp, T)) / (2.0 * dp);
    Real de_dT_fd = (_fp->e(p, T + dT) - _fp->e(p, T - dT)) / (2.0 * dT);
    Real e = 0.0, de_dp = 0.0, de_dT = 0.0;
    _fp->e_dpT(p, T, e, de_dp, de_dT);

    ABS_TEST("e", e, _fp->e(p, T), 1.0e-15);
    REL_TEST("de_dp", de_dp, de_dp_fd, tol);
    REL_TEST("de_dT", de_dT, de_dT_fd, tol);
  }

  std::shared_ptr<MooseApp> _app;
  std::unique_ptr<MooseMesh> _mesh;
  std::unique_ptr<FEProblem> _fe_problem;
  Factory * _factory;
  const Water97FluidProperties * _fp;
};

#endif // WATER97FLUIDPROPERTIESTEST_H
