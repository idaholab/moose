//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagTestProblem.h"
#include "NonlinearSystem.h"

registerMooseObject("MooseTestApp", TagTestProblem);

template <>
InputParameters
validParams<TagTestProblem>()
{
  InputParameters params = validParams<FEProblem>();

  MultiMooseEnum test_tagvs(" ", " ", true);
  MultiMooseEnum test_tagms(" ", " ", true);

  params.addParam<MultiMooseEnum>(
      "test_tag_vectors", test_tagvs, "Vector tags we are going to test against ");

  params.addParam<MultiMooseEnum>(
      "test_tag_matrices", test_tagms, "Matrix tags we are going to test against");

  return params;
}

TagTestProblem::TagTestProblem(const InputParameters & params) : FEProblem(params)
{
  auto & vectortags = params.get<MultiMooseEnum>("test_tag_vectors");

  vtags.clear();
  for (auto & vtag : vectortags)
    vtags.insert(vtag);

  auto & matrixtags = params.get<MultiMooseEnum>("test_tag_matrices");

  mtags.clear();
  for (auto & mtag : matrixtags)
    mtags.insert(mtag);
}

void
TagTestProblem::computeResidual(const NumericVector<Number> & soln,
                                NumericVector<Number> & residual)
{
  _fe_vector_tags.clear();

  for (auto & vtag : vtags)
    if (vectorTagExists(vtag))
    {
      auto tag = getVectorTagID(vtag);
      _fe_vector_tags.insert(tag);
    }
    else
      mooseError("Tag ", vtag, " does not exist");

  _nl->setSolution(soln);

  if (_fe_vector_tags.find(_nl->residualVectorTag()) != _fe_vector_tags.end())
    _nl->associateVectorToTag(residual, _nl->residualVectorTag());

  computeResidualTags(_fe_vector_tags);

  if (_fe_vector_tags.find(_nl->residualVectorTag()) != _fe_vector_tags.end())
    _nl->disassociateVectorFromTag(residual, _nl->residualVectorTag());
}

void
TagTestProblem::computeJacobian(const NumericVector<Number> & soln, SparseMatrix<Number> & jacobian)
{
  _fe_matrix_tags.clear();

  for (auto & mtag : mtags)
    if (matrixTagExists(mtag))
    {
      auto tag = getMatrixTagID(mtag);
      _fe_matrix_tags.insert(tag);
    }
    else
      mooseError("Tag ", mtag, " does not exist");

  _nl->setSolution(soln);

  if (_fe_matrix_tags.size() > 0)
    _nl->associateMatrixToTag(jacobian, *_fe_matrix_tags.begin());

  computeJacobianTags(_fe_matrix_tags);

  if (_fe_matrix_tags.size() > 0)
    _nl->disassociateMatrixFromTag(jacobian, *_fe_matrix_tags.begin());
}
