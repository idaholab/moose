//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "AutoCheckpointTest.h"
#include "MooseApp.h"
#include "OutputWarehouse.h"
#include "Checkpoint.h"

TEST_F(AutoCheckpointTest, noWallTimeIfCheckpointSetToFalse)
{
  // Check that wall time was not used
  const auto & output_names = _app->getOutputWarehouse().getOutputNames();
  // Checkpoint was created
  EXPECT_EQ(1, output_names.size());
  // No wall time though
  const std::string msg = "Wall Time Interval:      Disabled";
  const auto checkpoint_info = _app->getOutputWarehouse()
                                   .getOutput<Checkpoint>(*output_names.begin())
                                   ->checkpointInfo()
                                   .str();
  EXPECT_EQ(true, checkpoint_info.find(msg) != std::string::npos);
}
