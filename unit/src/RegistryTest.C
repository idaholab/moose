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
