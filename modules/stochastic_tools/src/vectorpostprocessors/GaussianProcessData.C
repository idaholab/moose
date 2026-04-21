//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes

#ifdef MOOSE_LIBTORCH_ENABLED

#include "GaussianProcessData.h"

registerMooseObject("StochasticToolsApp", GaussianProcessData);

namespace
{

bool
isScalarHyperParameter(const torch::Tensor & tensor)
{
  return tensor.dim() == 0;
}

bool
isVectorHyperParameter(const torch::Tensor & tensor)
{
  return tensor.dim() == 1;
}

std::vector<Real>
exportHyperParameter(const torch::Tensor & tensor)
{
  const auto flattened = tensor.reshape({-1}).contiguous();
  return {flattened.data_ptr<Real>(), flattened.data_ptr<Real>() + flattened.numel()};
}

} // namespace

InputParameters
GaussianProcessData::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params += SurrogateModelInterface::validParams();
  params.addClassDescription(
      "Tool for extracting hyperparameter data from gaussian process user object and "
      "storing in VectorPostprocessor vectors.");
  params.addRequiredParam<UserObjectName>("gp_name", "Name of GaussianProcess.");
  return params;
}

GaussianProcessData::GaussianProcessData(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SurrogateModelInterface(this),
    _gp_surrogate(getSurrogateModel<GaussianProcessSurrogate>("gp_name"))
{
}

void
GaussianProcessData::initialize()
{
  const auto & hyperparam_map = _gp_surrogate.getGP().getHyperParamMap();

  for (const auto & iter : hyperparam_map)
  {
    if (isScalarHyperParameter(iter.second))
    {
      _hp_vector.push_back(&declareVector(iter.first));
      _hp_vector.back()->push_back(iter.second.item<Real>());
      continue;
    }

    if (!isVectorHyperParameter(iter.second))
      mooseError("Unsupported hyperparameter rank ", iter.second.dim(), " for ", iter.first, ".");

    const auto vec = exportHyperParameter(iter.second);
    for (unsigned int ii = 0; ii < vec.size(); ++ii)
    {
      _hp_vector.push_back(&declareVector(iter.first + std::to_string(ii)));
      _hp_vector.back()->push_back(vec[ii]);
    }
  }
}

#endif
