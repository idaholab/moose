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

namespace libMesh
{
template <typename Number>
class PetscVector;
}

namespace OptUtils
{
void copyReporterIntoPetscVector(const std::vector<std::vector<Real> *> reporterVectors,
                                 libMesh::PetscVector<Number> & x);
void copyPetscVectorIntoReporter(const libMesh::PetscVector<Number> & x,
                                 std::vector<std::vector<Real> *> reporterVectors);
}
