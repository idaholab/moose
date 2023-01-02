//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

class InputParameters;
class MooseObject;
class Action;

#include <string>

namespace ThermochimicaUtils
{

/**
 * Add the supplied class description if thermochimica is available, otherwise add a warning
 * message.
 */
void addClassDescription(InputParameters & params, const std::string & desc);

/**
 * Check if thermochimica is available and throw an error if it is not.
 */
void checkLibraryAvailability(MooseObject & self);
void checkLibraryAvailability(Action & self);

}
