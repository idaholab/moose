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

    _app.reset(AppFactory::createApp("MooseUnitApp", 1, (char **)argv));
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

  std::unique_ptr<MooseApp> _app;
  std::unique_ptr<MooseMesh> _mesh;
  std::unique_ptr<FEProblem> _fe_problem;
  Factory * _factory;
};

#endif // USERFUNCTIONTEST_H
