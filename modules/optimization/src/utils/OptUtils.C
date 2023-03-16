//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptUtils.h"
#include "libmesh/petsc_vector.h"

// static functions to copy Petsc vectors to and from vectors for a Reporter

namespace OptUtils
{

void
copyReporterIntoPetscVector(const std::vector<std::vector<Real> *> reporterVectors,
                            libMesh::PetscVector<Number> & x)
{
  dof_id_type n = 0;
  for (const auto & data : reporterVectors)
    for (const auto & val : *data)
      x.set(n++, val);

  x.close();
}

void
copyPetscVectorIntoReporter(const libMesh::PetscVector<Number> & x,
                            std::vector<std::vector<Real> *> reporterVectors)
{
  dof_id_type n = 0;
  for (auto & data : reporterVectors)
    for (auto & val : *data)
      val = x(n++);
}

}
