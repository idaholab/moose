//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "Registry.h"

#include "Diffusion.h"
#include "MaterialRealAux.h"
#include "CheckOutputAction.h"

TEST(RegistryTest, getClassName)
{
  // This is a simple non-templated case
  EXPECT_EQ(Registry::getClassName<Diffusion>(), "Diffusion");

  // This is a templated case that would not work with demangle, as
  // demangle(typeid(ADMaterialRealAux).name()) returns
  // "MaterialRealAuxTempl<true>"
  EXPECT_EQ(Registry::getClassName<ADMaterialRealAux>(), "ADMaterialRealAux");

  // This tests the lookup of an action class name
  EXPECT_EQ(Registry::getClassName<CheckOutputAction>(), "CheckOutputAction");
}

TEST(RegistryTest, repositoryURL)
{
  const std::string repo_name = "bar";
  const std::string repo_url = "github.com/foo/bar";

  // not registered
  EXPECT_THROW(
      {
        try
        {
          Registry::getRepositoryURL(repo_name);
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "Registry::getRepositoryURL(): The repository '" + repo_name +
                        "' is not registered.");
          throw;
        }
      },
      std::exception);

  // register it
  Registry::addRepository(repo_name, repo_url);
  EXPECT_EQ(Registry::getRepositoryURL(repo_name), repo_url);

  // re-register, different URL
  EXPECT_THROW(
      {
        try
        {
          Registry::addRepository(repo_name, "badurl");
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "Registry::registerRepository(): The repository '" + repo_name +
                        "' is already registered "
                        "with a different URL '" +
                        repo_url + "'.");
          throw;
        }
      },
      std::exception);
}
