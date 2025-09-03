//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest/gtest.h"

#include "CSGCell.h"
#include "CSGUniverse.h"
#include "CSGRegion.h"
#include "CSGSphere.h"

#include "MooseUnitUtils.h"

namespace CSG
{

class CSGUniverseTest : public ::testing::Test
{
};

} // namespace CSG