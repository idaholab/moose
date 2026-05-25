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
/// Linear finite-volume gradient schemes supported by system-owned gradient fields.
enum class LinearFVGradientSchemeType
{
  GreenGauss
};

/// Type of gradient values stored in a linear FV gradient field.
enum class LinearFVGradientFieldType
{
  Base,
  Limited
};
}
}
