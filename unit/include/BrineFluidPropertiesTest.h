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
    const char * argv[] = {"foo", NULL};

    _app.reset(AppFactory::createApp("MooseUnitApp", 1, (char **)argv));
    _factory = &_app->getFactory();

    registerObjects(*_factory);
    buildObjects();
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
    _mesh = libmesh_make_unique<GeneratedMesh>(mesh_params);

    InputParameters problem_params = _factory->getValidParams("FEProblem");
    problem_params.set<MooseMesh *>("mesh") = _mesh.get();
    problem_params.set<std::string>("name") = "problem";
    problem_params.set<std::string>("_object_name") = "name2";
    _fe_problem = libmesh_make_unique<FEProblem>(problem_params);

    // The brine fluid properties
    InputParameters uo_pars = _factory->getValidParams("BrineFluidProperties");
    _fe_problem->addUserObject("BrineFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<BrineFluidProperties>("fp");

    // Get the water properties UserObject
    _water_fp = &_fp->getComponent(BrineFluidProperties::WATER);
  }

  std::unique_ptr<MooseApp> _app;
  std::unique_ptr<MooseMesh> _mesh;
  std::unique_ptr<FEProblem> _fe_problem;
  Factory * _factory;
  const BrineFluidProperties * _fp;
  const SinglePhaseFluidPropertiesPT * _water_fp;
};

#endif // BRINEFLUIDPROPERTIESTEST_H
