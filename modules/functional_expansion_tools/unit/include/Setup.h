//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "Cartesian.h"
#include "CylindricalDuo.h"
#include "Legendre.h"
#include "Zernike.h"

// Set the global tolerances
const double tol = 1e-13;

// Set the name
const std::string name = "UnitTesting";

// Recreate the MooseEnum types used in FunctionSeries::validParams()
extern MooseEnum single_series_types_1D;
extern MooseEnum single_series_types_2D;
extern MooseEnum expansion_type;
extern MooseEnum generation_type;
