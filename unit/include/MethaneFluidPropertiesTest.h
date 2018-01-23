//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef METHANEFLUIDPROPERTIESTEST_H
#define METHANEFLUIDPROPERTIESTEST_H

#include "gtest_include.h"

#include "FEProblem.h"
#include "AppFactory.h"
#include "GeneratedMesh.h"
#include "MethaneFluidProperties.h"
#include "MooseApp.h"
#include "Utils.h"

class MethaneFluidPropertiesTest : public ::testing::Test
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

  void registerObjects(Factory & factory) { registerUserObject(MethaneFluidProperties); }

  void buildObjects()
  {
    InputParameters mesh_params = _factory->getValidParams("GeneratedMesh");
    mesh_params.set<MooseEnum>("dim") = "3";
    mesh_params.set<std::string>("name") = "mesh";
    mesh_params.set<std::string>("_object_name") = "name1";
    _mesh = libmesh_make_unique<GeneratedMesh>(mesh_params);

    InputParameters problem_params = _factory->getValidParams("FEProblem");
    problem_params.set<MooseMesh *>("mesh") = _mesh.get();
    problem_params.set<std::string>("_object_name") = "name2";
    auto fep = _factory->create<FEProblemBase>("FEProblem", "problem", problem_params);

    InputParameters uo_pars = _factory->getValidParams("MethaneFluidProperties");
    fep->addUserObject("MethaneFluidProperties", "fp", uo_pars);
    _fp = &fep->getUserObject<MethaneFluidProperties>("fp");
  }

  std::unique_ptr<MooseMesh> _mesh; // mesh must destruct last and so be declared first
  std::shared_ptr<MooseApp> _app;
  Factory * _factory;
  const MethaneFluidProperties * _fp;
};

#endif // METHANEFLUIDPROPERTIESTEST_H
