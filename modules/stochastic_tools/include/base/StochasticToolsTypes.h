//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/// Enum for batch type in stochastic tools MultiApp
namespace StochasticTools
{
enum class MultiAppMode
{
  NORMAL = 0,
  BATCH_RESET = 1,
  BATCH_RESTORE = 2
};
}
