//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

namespace Moose
{
namespace FV
{
/**
 * Cell-gradient limiter variants used for MUSCL-style reconstructions.
 *
 * This is intentionally separate from face/value limiter selections. Gradient limiting typically
 * has fewer supported options and uses different data/loops.
 */
enum class GradientLimiterType : int
{
  None = -1,
  Venkatakrishnan = 0
};
}
}

