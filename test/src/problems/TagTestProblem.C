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

InputParameters
TagTestProblem::validParams()
{
  InputParameters params = FEProblem::validParams();

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
                                NumericVector<Number> & residual,
                                const unsigned int nl_sys_num)
{
  setCurrentNonlinearSystem(nl_sys_num);
  _fe_vector_tags.clear();

  for (auto & vtag : vtags)
    if (vectorTagExists(vtag))
    {
      auto tag = getVectorTagID(vtag);
      _fe_vector_tags.insert(tag);
    }
    else
      mooseError("Tag ", vtag, " does not exist");

  getNonlinearSystemBase(nl_sys_num).setSolution(soln);

  if (_fe_vector_tags.find(getNonlinearSystemBase(nl_sys_num).residualVectorTag()) !=
      _fe_vector_tags.end())
    getNonlinearSystemBase(nl_sys_num)
        .associateVectorToTag(residual, getNonlinearSystemBase(nl_sys_num).residualVectorTag());

  computeResidualTags(_fe_vector_tags);

  if (_fe_vector_tags.find(getNonlinearSystemBase(nl_sys_num).residualVectorTag()) !=
      _fe_vector_tags.end())
    getNonlinearSystemBase(nl_sys_num)
        .disassociateVectorFromTag(residual,
                                   getNonlinearSystemBase(nl_sys_num).residualVectorTag());
}

void
TagTestProblem::computeJacobian(const NumericVector<Number> & soln,
                                SparseMatrix<Number> & jacobian,
                                const unsigned int nl_sys_num)
{
  setCurrentNonlinearSystem(nl_sys_num);
  _fe_matrix_tags.clear();

  for (auto & mtag : mtags)
    if (matrixTagExists(mtag))
    {
      auto tag = getMatrixTagID(mtag);
      _fe_matrix_tags.insert(tag);
    }
    else
      mooseError("Tag ", mtag, " does not exist");

  getNonlinearSystemBase(nl_sys_num).setSolution(soln);

  if (_fe_matrix_tags.size() > 0)
    getNonlinearSystemBase(nl_sys_num).associateMatrixToTag(jacobian, *_fe_matrix_tags.begin());

  computeJacobianTags(_fe_matrix_tags);

  if (_fe_matrix_tags.size() > 0)
    getNonlinearSystemBase(nl_sys_num)
        .disassociateMatrixFromTag(jacobian, *_fe_matrix_tags.begin());
}
