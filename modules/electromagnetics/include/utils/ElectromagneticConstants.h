//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/**
 *  ElectromagneticConstants contains various constants useful in the
 *  Electromagnetic module, such as the complex number j, vacuum electric
 *  permittivity, etc.
 */
namespace EM
{
/// Complex number "j" (also known as "i")
static const std::complex<double> j(0, 1);
} // namespace EM
