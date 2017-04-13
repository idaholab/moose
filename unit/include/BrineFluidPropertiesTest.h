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

#ifndef BRINEFLUIDPROPERTIESTEST_H
#define BRINEFLUIDPROPERTIESTEST_H

#include "gtest/gtest.h"

#include "MooseApp.h"
#include "Utils.h"
#include "FEProblem.h"
#include "AppFactory.h"
#include "GeneratedMesh.h"
#include "BrineFluidProperties.h"
#include "Water97FluidProperties.h"
#include "NaClFluidProperties.h"

class MooseMesh;
class FEProblem;
class BrineFluidProperties;
class SinglePhaseFluidPropertiesPT;

class BrineFluidPropertiesTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    char str[] = "foo";
    char * argv[] = {str, NULL};

    _app = AppFactory::createApp("MooseUnitApp", 1, (char **)argv);
    _factory = &_app->getFactory();

    registerObjects(*_factory);
    buildObjects();
  }

  void TearDown()
  {
    delete _fe_problem;
    delete _mesh;
    delete _app;
  }

  void registerObjects(Factory & factory)
  {
    registerUserObject(BrineFluidProperties);
    registerUserObject(Water97FluidProperties);
    registerUserObject(NaClFluidProperties);
  }

  void buildObjects()
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

  MooseApp * _app;
  Factory * _factory;
  MooseMesh * _mesh;
  FEProblem * _fe_problem;
  const BrineFluidProperties * _fp;
  const SinglePhaseFluidPropertiesPT * _water_fp;
};

#endif // BRINEFLUIDPROPERTIESTEST_H
