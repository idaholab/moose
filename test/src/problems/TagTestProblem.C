//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagTestProblem.h"
#include "NonlinearSystem.h"
#include "LinearSystem.h"

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

  _vtags.clear();
  for (auto & vtag : vectortags)
    _vtags.insert(vtag);

  auto & matrixtags = params.get<MultiMooseEnum>("test_tag_matrices");

  _mtags.clear();
  for (auto & mtag : matrixtags)
    _mtags.insert(mtag);
}

void
TagTestProblem::computeResidual(const NumericVector<Number> & soln,
                                NumericVector<Number> & residual,
                                const unsigned int nl_sys_num)
{
  setCurrentNonlinearSystem(nl_sys_num);
  _fe_vector_tags.clear();

  for (auto & vtag : _vtags)
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

  for (auto & mtag : _mtags)
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

void
TagTestProblem::computeLinearSystemSys(LinearImplicitSystem & sys,
                                       SparseMatrix<Number> & system_matrix,
                                       NumericVector<Number> & rhs,
                                       const bool compute_gradients)
{
  const auto linear_sys_num = linearSysNum(sys.name());
  auto & linear_sys = getLinearSystem(linear_sys_num);
  setCurrentLinearSystem(linearSysNum(sys.name()));

  std::vector<VectorTag> vector_tags;
  for (const auto & vtag : _vtags)
    if (vectorTagExists(vtag))
      vector_tags.push_back(getVectorTag(getVectorTagID(vtag)));
    else
      mooseError("Tag ", vtag, " does not exist");

  for (const auto & tag : vector_tags)
    linear_sys.associateVectorToTag(rhs, tag._id);

  std::set<TagID> selected_vtags;
  selectVectorTagsFromSystem(linear_sys, vector_tags, selected_vtags);

  std::map<TagName, TagID> matrix_tags;
  for (auto & mtag : _mtags)
    if (matrixTagExists(mtag))
      matrix_tags.insert(std::make_pair(mtag, getMatrixTagID(mtag)));
    else
      mooseError("Tag ", mtag, " does not exist");

  for (const auto & tag : matrix_tags)
    linear_sys.associateMatrixToTag(system_matrix, tag.second);

  std::set<TagID> selected_mtags;
  selectMatrixTagsFromSystem(linear_sys, matrix_tags, selected_mtags);

  computeLinearSystemTags(
      *(_current_linear_sys->currentSolution()), selected_vtags, selected_mtags, compute_gradients);
}
