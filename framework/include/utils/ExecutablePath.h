//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * Gets the full path to the running executable on
 * Mac OS X and linux.
 *
 * Not implemented for windows.
 */
std::string getExec();

/**
 * Gets the directory the running executable is on
 * Mac OS X and linux.
 *
 * Not implemented for windows.
 */
std::string getExecutablePath();

/**
 * Gets the name of the running executable on
 * Mac OS X and linux.
 *
 * Not implemented for windows.
 */
std::string getExecutableName();
}
