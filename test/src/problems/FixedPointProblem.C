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

InputParameters
FixedPointProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
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

  const auto & residual_vector_tags = getVectorTags(Moose::VECTOR_TAG_RESIDUAL);

  _fe_vector_tags.clear();

  for (const auto & residual_vector_tag : residual_vector_tags)
    if (residual_vector_tag._id != _tag_id)
      _fe_vector_tags.insert(residual_vector_tag._id);

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
