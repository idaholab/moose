//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParallelAcquisitionFunctionBase.h"

InputParameters
ParallelAcquisitionFunctionBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addClassDescription("Base class for parallel acquisition functions");
  params.registerBase("ParallelAcquisitionFunctionBase");
  params.registerSystemAttributeName("ParallelAcquisitionFunctionBase");
  return params;
}

ParallelAcquisitionFunctionBase::ParallelAcquisitionFunctionBase(const InputParameters & parameters)
  : MooseObject(parameters)
{
}

void
ParallelAcquisitionFunctionBase::computeAcquisition(
    std::vector<Real> & acq,
    const std::vector<Real> & gp_mean,
    const std::vector<Real> & gp_std,
    const std::vector<std::vector<Real>> & test_inputs,
    const std::vector<std::vector<Real>> & train_inputs,
    const std::vector<Real> & generic) const
{
  // Size checks
  mooseAssert(test_inputs.size() == 0, "computeAcquisition: test_inputs must be non-empty.");

  mooseAssert(gp_mean.size() != test_inputs.size() || gp_std.size() != test_inputs.size(),
              "computeAcquisition: gp_mean, gp_std, and test_inputs must have the same length.");

  mooseAssert(acq.size() != test_inputs.size(),
              "computeAcquisition: output 'acq' must be pre-sized to test_inputs.size().");

  // Dimensionality checks (all rows same dim; train dim matches test dim)
  mooseAssert(test_inputs.front().size() == 0,
              "computeAcquisition: test_inputs row dimension must be > 0.");

  mooseAssert(train_inputs[0].size() != test_inputs.front().size(),
              "computeAcquisition: train_inputs rows must match test_inputs dimension.");

  // Hand off to the derived implementation
  computeAcquisitionInternal(acq, gp_mean, gp_std, test_inputs, train_inputs, generic);
}

void
ParallelAcquisitionFunctionBase::penalizeAcquisition(std::vector<Real> & modified_acq,
                                                     std::vector<unsigned int> & sorted_indices,
                                                     const std::vector<Real> & acq,
                                                     const std::vector<Real> & length_scales,
                                                     const std::vector<std::vector<Real>> & inputs)
{
  mooseAssert(inputs.size() == 0, "penalizeAcquisition: 'inputs' must be non-empty.");
  mooseAssert(
      modified_acq.size() != inputs.size() || sorted_indices.size() != inputs.size() ||
          acq.size() != inputs.size(),
      "penalizeAcquisition: modified_acq, sorted_indices, and acq must match inputs.size().");

  mooseAssert(inputs.front().size() != inputs.front().size(),
              "penalizeAcquisition: all input rows must have the same dimension.");
  mooseAssert(length_scales.size() != inputs.front().size(),
              "penalizeAcquisition: length_scales must match input dimension.");

  std::vector<Real> negate_acq = acq;
  std::transform(negate_acq.cbegin(), negate_acq.cend(), negate_acq.begin(), std::negate<double>());
  std::vector<size_t> ind;
  Moose::indirectSort(negate_acq.begin(), negate_acq.end(), ind);
  modified_acq[0] = -negate_acq[ind[0]];
  sorted_indices[0] = ind[0];

  Real correlation = 0.0;
  for (unsigned int i = 0; i < inputs.size() - 1; ++i)
  {
    for (unsigned int j = 0; j < inputs.size(); ++j)
    {
      computeCorrelation(correlation, inputs[j], inputs[ind[0]], length_scales);
      negate_acq[j] = negate_acq[j] * correlation;
    }
    Moose::indirectSort(negate_acq.begin(), negate_acq.end(), ind);
    modified_acq[i + 1] = -negate_acq[ind[0]];
    sorted_indices[i + 1] = ind[0];
  }
}

void
ParallelAcquisitionFunctionBase::computeCorrelation(Real & corr,
                                                    const std::vector<Real> & input1,
                                                    const std::vector<Real> & input2,
                                                    const std::vector<Real> & length_scales)
{
  mooseAssert(input1.size() != input2.size() || input1.size() != length_scales.size(),
              "computeCorrelation: input1, input2, and length_scales must be the same size.");
  corr = 0.0;
  for (unsigned int i = 0; i < input1.size(); ++i)
    corr -= Utility::pow<2>(input1[i] - input2[i]) / (2.0 * Utility::pow<2>(length_scales[i]));
  corr = 1.0 - std::exp(corr);
}
