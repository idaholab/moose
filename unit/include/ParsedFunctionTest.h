//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef USERFUNCTIONTEST_H
#define USERFUNCTIONTEST_H

#include "gtest/gtest.h"

#include "InputParameters.h"
#include "MooseParsedFunction.h"
#include "FEProblem.h"
#include "MooseUnitApp.h"
#include "AppFactory.h"
#include "GeneratedMesh.h"
#include "MooseParsedFunctionWrapper.h"

class ParsedFunctionTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    const char * argv[2] = {"foo", "\0"};

    _app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
    _factory = &_app->getFactory();

    InputParameters mesh_params = _factory->getValidParams("GeneratedMesh");
    mesh_params.set<MooseEnum>("dim") = "3";
    mesh_params.set<std::string>("_object_name") = "mesh";
    _mesh = libmesh_make_unique<GeneratedMesh>(mesh_params);

    InputParameters problem_params = _factory->getValidParams("FEProblem");
    problem_params.set<MooseMesh *>("mesh") = _mesh.get();
    problem_params.set<std::string>("_object_name") = "FEProblem";
    _fe_problem = libmesh_make_unique<FEProblem>(problem_params);
  }

  ParsedFunction<Real> * fptr(MooseParsedFunction & f)
  {
    return f._function_ptr->_function_ptr.get();
  }

  std::shared_ptr<MooseApp> _app;
  std::unique_ptr<MooseMesh> _mesh;
  std::unique_ptr<FEProblem> _fe_problem;
  Factory * _factory;
};

#endif // USERFUNCTIONTEST_H
