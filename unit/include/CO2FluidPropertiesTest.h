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

#ifndef CO2FLUIDPROPERTIESTEST_H
#define CO2FLUIDPROPERTIESTEST_H

#include "gtest/gtest.h"

#include "FEProblem.h"
#include "AppFactory.h"
#include "GeneratedMesh.h"
#include "CO2FluidProperties.h"
#include "MooseApp.h"

class MooseMesh;
class FEProblem;
class CO2FluidProperties;

class CO2FluidPropertiesTest : public ::testing::Test
{
protected:
  void registerObjects(Factory & factory) { registerUserObject(CO2FluidProperties); }

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

    InputParameters uo_pars = _factory->getValidParams("CO2FluidProperties");
    _fe_problem->addUserObject("CO2FluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<CO2FluidProperties>("fp");
  }

  void SetUp()
  {
    const char * argv[] = {"foo", NULL};

    _app.reset(AppFactory::createApp("MooseUnitApp", 1, (char **)argv));
    _factory = &_app->getFactory();

    registerObjects(*_factory);
    buildObjects();
  }

  std::unique_ptr<MooseApp> _app;
  std::unique_ptr<MooseMesh> _mesh;
  std::unique_ptr<FEProblem> _fe_problem;
  Factory * _factory;
  const CO2FluidProperties * _fp;
};

#endif // CO2FLUIDPROPERTIESTEST_H
