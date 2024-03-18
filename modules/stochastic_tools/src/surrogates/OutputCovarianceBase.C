//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OutputCovarianceBase.h"

InputParameters
OutputCovarianceBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addClassDescription("Base class for output covariance functions");
  params.registerBase("OutputCovarianceBase");
  params.registerSystemAttributeName("OutputCovarianceBase");
  return params;
}

OutputCovarianceBase::OutputCovarianceBase(const InputParameters & parameters)
  : MooseObject(parameters)
{
}

unsigned int
OutputCovarianceBase::setupNumLatent(const unsigned int & /*num_outputs*/) const
{
  return 0;
}

void
OutputCovarianceBase::computeFullCovarianceMatrix(RealEigenMatrix & kappa,
                                                  const RealEigenMatrix & B,
                                                  const RealEigenMatrix & K) const
{
  kron(B, K, kappa);
}

void
OutputCovarianceBase::kron(const RealEigenMatrix & mat_A,
                           const RealEigenMatrix & mat_B,
                           RealEigenMatrix & mat_req) const
{
  mat_req.resize(mat_A.rows() * mat_B.rows(), mat_A.cols() * mat_B.cols());
  for (unsigned int i = 0; i < mat_A.rows(); i++)
    for (unsigned int j = 0; j < mat_A.cols(); j++)
      for (unsigned int k = 0; k < mat_B.rows(); k++)
        for (unsigned int l = 0; l < mat_B.cols(); l++)
          mat_req(((i * mat_B.rows()) + k), ((j * mat_B.cols()) + l)) = mat_A(i, j) * mat_B(k, l);
}
