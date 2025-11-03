//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WebServerControlTypeRegistration.h"

#include <string>

#include "libmesh/parallel_eigen.h"

/**
 * Registers the base parameter types that are controllable
 * in the WebServerControl.
 *
 * You can use the common macros from the header file
 * to register custom types as controllable in your
 * own application or derived WebServerControl.
 */
namespace Moose::WebServerControlTypeRegistration
{
// Scalar types
registerWebServerControlScalar(bool);
registerWebServerControlScalar(Real);
registerWebServerControlScalar(int);
registerWebServerControlScalar(std::string);

// Vector types
registerWebServerControlVector(Real);
registerWebServerControlVector(int);
registerWebServerControlVector(std::string);

// RealEigenMatrix
registerWebServerControlType(RealEigenMatrix,
                             [](const nlohmann::json & json_value) -> RealEigenMatrix
                             {
                               const auto vec_of_vec =
                                   json_value.get<std::vector<std::vector<double>>>();
                               const auto nrows = vec_of_vec.size();
                               if (nrows == 0)
                                 return RealEigenMatrix::Zero(0, 0);
                               const auto ncols = vec_of_vec[0].size();

                               RealEigenMatrix matrix;
                               matrix.resize(nrows, ncols);
                               for (const auto i : make_range(nrows))
                               {
                                 const auto & row = vec_of_vec[i];
                                 if (row.size() != ncols)
                                   throw std::runtime_error("Matrix is jagged");

                                 for (const auto j : index_range(row))
                                   matrix(i, j) = row[j];
                               }

                               return matrix;
                             });
}
