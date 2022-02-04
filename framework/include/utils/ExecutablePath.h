//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>

namespace Moose
{

/**
 * This function returns the PATH of the running executable.
 * There is not a portable way to do this function implements
 * this function for Mac OS X and Linux boxes containing a
 * normal /proc tree
 */
std::string getExecutablePath();

/**
 * This function returns the name of the running executable.
 * There is not a portable way to do this function implements
 * this function for Mac OS X and Linux boxes containing a
 * normal /proc tree
 */
std::string getExecutableName();
}
