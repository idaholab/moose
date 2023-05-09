//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "nlohmann/json.h"

class MooseApp;

// Specializations for to_json, which _must_ be specialized in the namespace
// in which the object is found
void to_json(nlohmann::json & json, const MooseApp & app); // MooseDocs:to_json
