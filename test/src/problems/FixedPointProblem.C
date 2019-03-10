//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FixedPointProblem.h"

#include "NonlinearSystemBase.h"

registerMooseObject("MooseTestApp", FixedPointProblem);

template <>
InputParameters
validParams<FixedPointProblem>()
{
  InputParameters params = validParams<FEProblem>();
  params.addParam<TagName>("fp_tag_name", "fp_previous", "Tag name for the fixed point iteration");
  return params;
}

FixedPointProblem::FixedPointProblem(const InputParameters & params)
  : FEProblem(params),
    _tag_previous(getParam<TagName>("fp_tag_name")),
    _tag_id(addVectorTag(_tag_previous)),
    _residual_previous(_nl->addVector(_tag_id, false, GHOSTED))
{
}

void
FixedPointProblem::computeResidual(const NumericVector<Number> & soln,
                                   NumericVector<Number> & residual)
{
  // excluding the previous tag evaluation
  _nl->disassociateVectorFromTag(_residual_previous, _tag_id);

  auto & tags = getVectorTags();

  _fe_vector_tags.clear();

  for (auto & tag : tags)
    if (tag.second != _tag_id)
      _fe_vector_tags.insert(tag.second);

  computeResidualInternal(soln, residual, _fe_vector_tags);

  residual += _residual_previous;

  _nl->associateVectorToTag(_residual_previous, _tag_id);
}

void
FixedPointProblem::computeFullResidual(const NumericVector<Number> & soln,
                                       NumericVector<Number> & residual)
{
  FEProblem::computeResidual(soln, residual);
  residual += _residual_previous;
}
