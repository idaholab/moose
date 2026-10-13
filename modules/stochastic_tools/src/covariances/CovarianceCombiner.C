//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CovarianceCombiner.h"

registerMooseObject("StochasticToolsApp", CovarianceCombiner);

InputParameters
CovarianceCombiner::validParams()
{
  InputParameters params = CovarianceFunctionBase::validParams();

  params.addClassDescription(
      "Composite covariance function that combines exactly two child kernels. "
      "'Sum' computes K = K1 + K2; 'Product' computes K = K1 .* K2 (Hadamard). "
      "All hyperparameters of both sub-kernels are exposed and tuned simultaneously. "
      "For Product, set noise_variance = 0 in both sub-kernels (see class docs).");

  params.setDocString("covariance_functions",
                      "Exactly two covariance function names to combine, given as "
                      "[kernel_1 kernel_2]. Either child may itself be a CovarianceCombiner "
                      "to build arbitrarily deep kernel trees.");

  MooseEnum operation_options("Sum Product");
  params.addRequiredParam<MooseEnum>(
      "operation",
      operation_options,
      "The combination rule applied to the two child kernels:\n"
      "  'Sum'     -- K = K1 + K2 (additive; each sub-kernel retains its own noise)\n"
      "  'Product' -- K = K1 .* K2 (element-wise; sub-kernel noise_variance should be 0)");

  return params;
}

CovarianceCombiner::CovarianceCombiner(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters),
    _operation(getParam<MooseEnum>("operation") == "Sum" ? Operation::Sum : Operation::Product)
{
  // The base class populates _covariance_functions from the 'covariance_functions' param.
  if (_covariance_functions.size() != 2)
    mooseError("CovarianceCombiner requires exactly two entries in 'covariance_functions', "
               "but ",
               _covariance_functions.size(),
               " were provided.");
}

void
CovarianceCombiner::computeCovarianceMatrix(RealEigenMatrix & K,
                                             const RealEigenMatrix & x,
                                             const RealEigenMatrix & xp,
                                             const bool is_self_covariance) const
{
  switch (_operation)
  {
    case Operation::Sum:
    {
      _covariance_functions[0]->computeCovarianceMatrix(K, x, xp, is_self_covariance);

      RealEigenMatrix K2(x.rows(), xp.rows());
      _covariance_functions[1]->computeCovarianceMatrix(K2, x, xp, is_self_covariance);

      K += K2;
      break;
    }


    case Operation::Product:
    {
      _covariance_functions[0]->computeCovarianceMatrix(K, x, xp, false);

      RealEigenMatrix K2(x.rows(), xp.rows());
      _covariance_functions[1]->computeCovarianceMatrix(K2, x, xp, false);

      K = K.cwiseProduct(K2);
      break;
    }
  }
}

bool
CovarianceCombiner::computedKdhyper(RealEigenMatrix & dKdhp,
                                     const RealEigenMatrix & x,
                                     const std::string & hyper_param_name,
                                     unsigned int ind) const
{

  switch (_operation)
  {

    case Operation::Sum:
    {
      if (_covariance_functions[0]->computedKdhyper(dKdhp, x, hyper_param_name, ind))
        return true;
      if (_covariance_functions[1]->computedKdhyper(dKdhp, x, hyper_param_name, ind))
        return true;
      return false;
    }

    case Operation::Product:
    {
      const unsigned int n = x.rows();
      RealEigenMatrix dK_sub(n, n);

      if (_covariance_functions[0]->computedKdhyper(dK_sub, x, hyper_param_name, ind))
      {
        // dK/dhp1 = (dK1/dhp1) .* K2_base
        RealEigenMatrix K2(n, n);
        _covariance_functions[1]->computeCovarianceMatrix(K2, x, x, false);
        dKdhp = dK_sub.cwiseProduct(K2);
        return true;
      }

      if (_covariance_functions[1]->computedKdhyper(dK_sub, x, hyper_param_name, ind))
      {
        // dK/dhp2 = K1_base .* (dK2/dhp2)
        RealEigenMatrix K1(n, n);
        _covariance_functions[0]->computeCovarianceMatrix(K1, x, x, false);
        dKdhp = K1.cwiseProduct(dK_sub);
        return true;
      }

      return false;
    }
  }

  return false;
}
