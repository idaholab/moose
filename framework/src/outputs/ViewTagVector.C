//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ViewTagVector.h"

// MOOSE includes
#include "NonlinearSystemBase.h"
#include "FEProblemBase.h"

#include "libmesh/libmesh_config.h"
#include "libmesh/sparse_matrix.h"

template <>
InputParameters
validParams<ViewTagVector>()
{
  InputParameters params = validParams<PetscOutput>();

  MultiMooseEnum extra_tagvs(" ", " ", true);
  MultiMooseEnum extra_tagms(" ", " ", true);

  params.addParam<MultiMooseEnum>(
      "view_tag_vectors", extra_tagvs, "The vectors will be visualized");

  params.addParam<MultiMooseEnum>(
      "view_tag_matrices", extra_tagms, "The matrices will be visualized");

  return params;
}

ViewTagVector::ViewTagVector(const InputParameters & parameters)
  : PetscOutput(parameters), _nl(_problem_ptr->getNonlinearSystemBase())
{
}

void
ViewTagVector::output(const ExecFlagType & /*type*/)
{
  // visualize vectors
  auto & vectors = getParam<MultiMooseEnum>("view_tag_vectors");

  for (auto & vector : vectors)
  {
    if (_problem_ptr->vectorTagExists(vector.name()))
    {
      auto tagid = _problem_ptr->getVectorTagID(vector.name());

      if (_nl.hasVector(tagid))
        _nl.getVector(tagid).print();
    }
  }

  // visualize matrices
  auto & matrices = getParam<MultiMooseEnum>("view_tag_matrices");

  for (auto & matrix : matrices)
  {
    if (_problem_ptr->matrixTagExists(matrix.name()))
    {
      auto tagid = _problem_ptr->getMatrixTagID(matrix.name());

      if (_nl.hasMatrix(tagid))
      {
        if (_nl.getMatrix(tagid).closed())
          _nl.getMatrix(tagid).print();
      }
    }
  }
}
