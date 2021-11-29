//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// CPPUnit includes
#include "gtest_include.h"

// Moose includes
#include "EBSDMeshGenerator.h"
#include "InputParameters.h"
#include "MooseParsedFunction.h"
#include "PhaseFieldApp.h"
#include "AppFactory.h"

class EBSDMeshErrorTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    const char * argv[2] = {"foo", "\0"};
    _app = AppFactory::createAppShared("PhaseFieldApp", 1, (char **)argv);
    _factory = &_app->getFactory();
  }

  template <typename T>
  void testParam(unsigned int nparam, const char ** param_list, std::string name)
  {
    for (unsigned int i = 0; i < nparam; ++i)
    {
      // create a unique name
      std::ostringstream oss;
      oss << name << "_" << i;

      // generate input parameter set
      InputParameters params = EBSDMeshGenerator::validParams();
      params.addPrivateParam("_moose_app", _app.get());
      params.set<std::string>("_object_name") = oss.str();
      params.set<std::string>("_type") = "EBSDMeshGenerator";

      // set a single parameter
      params.set<T>(param_list[i]) = T(1.0);

      // set filename (is a required param but not used in these tests)
      params.set<FileName>("filename") = "DUMMY";

      try
      {
        // construct mesh object
        auto mesh = std::make_unique<EBSDMeshGenerator>(params);
        // TODO: fix and uncomment this - it was missing before.
        // FAIL() << "mesh construction should have failed but didn't";
      }
      catch (const std::exception & e)
      {
        std::string msg(e.what());
        ASSERT_TRUE(
            msg.find("Do not specify mesh geometry information, it is read from the EBSD file.") !=
            std::string::npos)
            << "failed with unexpected error: " << msg;
      }
    }
  }

  std::shared_ptr<MooseApp> _app;
  Factory * _factory;
};
