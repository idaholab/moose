//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TABULATEDFLUIDPROPERTIESTEST_H
#define TABULATEDFLUIDPROPERTIESTEST_H

#include "gtest/gtest.h"

#include "FEProblem.h"
#include "AppFactory.h"
#include "GeneratedMesh.h"
#include "TabulatedFluidProperties.h"
#include "CO2FluidProperties.h"
#include "MooseApp.h"

class MooseMesh;
class FEProblem;
class CO2FluidProperties;
class TabulatedFluidProperties;

class TabulatedFluidPropertiesTest : public ::testing::Test
{
protected:
  void registerObjects(Factory & factory)
  {
    registerUserObject(CO2FluidProperties);
    registerUserObject(TabulatedFluidProperties);
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

    InputParameters co2_uo_params = _factory->getValidParams("CO2FluidProperties");
    _fe_problem->addUserObject("CO2FluidProperties", "co2_fp", co2_uo_params);
    _co2_fp = &_fe_problem->getUserObject<CO2FluidProperties>("co2_fp");

    InputParameters tab_uo_params = _factory->getValidParams("TabulatedFluidProperties");
    tab_uo_params.set<UserObjectName>("fp") = "co2_fp";
    tab_uo_params.set<FileName>("fluid_property_file") = "data/csv/fluid_props.csv";
    _fe_problem->addUserObject("TabulatedFluidProperties", "tab_fp", tab_uo_params);
    _tab_fp = &_fe_problem->getUserObject<TabulatedFluidProperties>("tab_fp");

    InputParameters unordered_uo_params = _factory->getValidParams("TabulatedFluidProperties");
    unordered_uo_params.set<UserObjectName>("fp") = "co2_fp";
    unordered_uo_params.set<FileName>("fluid_property_file") = "data/csv/unordered_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedFluidProperties", "unordered_fp", unordered_uo_params);
    _unordered_fp = &_fe_problem->getUserObject<TabulatedFluidProperties>("unordered_fp");

    InputParameters unequal_uo_params = _factory->getValidParams("TabulatedFluidProperties");
    unequal_uo_params.set<UserObjectName>("fp") = "co2_fp";
    unequal_uo_params.set<FileName>("fluid_property_file") = "data/csv/unequal_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedFluidProperties", "unequal_fp", unequal_uo_params);
    _unequal_fp = &_fe_problem->getUserObject<TabulatedFluidProperties>("unequal_fp");

    InputParameters missing_col_uo_params = _factory->getValidParams("TabulatedFluidProperties");
    missing_col_uo_params.set<UserObjectName>("fp") = "co2_fp";
    missing_col_uo_params.set<FileName>("fluid_property_file") =
        "data/csv/missing_col_fluid_props.csv";
    _fe_problem->addUserObject("TabulatedFluidProperties", "missing_col_fp", missing_col_uo_params);
    _missing_col_fp = &_fe_problem->getUserObject<TabulatedFluidProperties>("missing_col_fp");

    InputParameters missing_data_uo_params = _factory->getValidParams("TabulatedFluidProperties");
    missing_data_uo_params.set<UserObjectName>("fp") = "co2_fp";
    missing_data_uo_params.set<FileName>("fluid_property_file") =
        "data/csv/missing_data_fluid_props.csv";
    _fe_problem->addUserObject(
        "TabulatedFluidProperties", "missing_data_fp", missing_data_uo_params);
    _missing_data_fp = &_fe_problem->getUserObject<TabulatedFluidProperties>("missing_data_fp");
  }

  void SetUp()
  {
    const char * argv[] = {"foo", NULL};

    _app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
    _factory = &_app->getFactory();

    registerObjects(*_factory);
    buildObjects();
  }

  std::shared_ptr<MooseApp> _app;
  std::unique_ptr<MooseMesh> _mesh;
  std::unique_ptr<FEProblem> _fe_problem;
  Factory * _factory;
  const CO2FluidProperties * _co2_fp;
  const TabulatedFluidProperties * _tab_fp;
  const TabulatedFluidProperties * _unordered_fp;
  const TabulatedFluidProperties * _unequal_fp;
  const TabulatedFluidProperties * _missing_col_fp;
  const TabulatedFluidProperties * _missing_data_fp;
};

#endif // TABULATEDFLUIDPROPERTIESTEST_H
