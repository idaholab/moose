//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "DenseMatrix.h"
#include <libmesh/dense_vector.h>

namespace GeochemistryFormattedOutput
{
/**
 * Returns a nicely formatted string corresponding to the reaction defined by the given row of the
 * stoichiometric matrix. For instance, if stoi(row, :) = (1, 2, 1E-5, -0.333333333333) and names =
 * ["H2O", "H+", "HCO3-", "O2(aq)"] then this function will return "H2O + 2*H+ - 0.333*O2(aq)" if
 * stoi_tol=1E-4 and precision = 3
 * @param stoi matrix stoichiometric coefficients
 * @param row row of stoi which defines the reaction of interest
 * @param names names of the basis species
 * @param stoi_tol Any stoichiometric coefficients of magnitude less than this value will not appear
 * in the returned string
 * @param precision Maximum number of digits written for each stoichiometric coefficient in the
 * returned string
 */
std::string reaction(const DenseMatrix<Real> & stoi,
                     unsigned row,
                     const std::vector<std::string> & names,
                     Real stoi_tol = 1.0E-6,
                     int precision = 4);
}
