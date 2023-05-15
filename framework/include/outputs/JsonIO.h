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
#include "libmesh/dense_vector.h"

class MooseApp;
namespace libMesh
{
class Point;
}
class VariableName;

// Overloads for to_json, which _must_ be overloaded in the namespace
// in which the object is found in order to enable argument-dependent lookup.
// See https://en.cppreference.com/w/cpp/language/adl for more information
void to_json(nlohmann::json & json, const MooseApp & app); // MooseDocs:to_json

namespace libMesh
{
void to_json(nlohmann::json & json, const Point & p);
void to_json(nlohmann::json & json, const DenseVector<Real> & vector);
}
