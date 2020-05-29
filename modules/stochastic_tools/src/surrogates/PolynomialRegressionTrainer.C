//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolynomialRegressionTrainer.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", PolynomialRegressionTrainer);

InputParameters
PolynomialRegressionTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();
  params.addClassDescription("Computes coefficients for linear regession model.");
  params.addRequiredParam<SamplerName>("sampler", "Training set defined by a sampler object.");
  params.addRequiredParam<VectorPostprocessorName>(
      "results_vpp", "Vectorpostprocessor with results of samples created by trainer.");
  params.addRequiredParam<std::string>(
      "results_vector",
      "Name of vector from vectorpostprocessor with results of samples created by trainer");

  return params;
}

PolynomialRegressionTrainer::PolynomialRegressionTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _coeff(declareModelData<std::vector<Real>>("_coeff"))
{
}

void
PolynomialRegressionTrainer::initialSetup()
{
  // Results VPP
  _values_distributed = isVectorPostprocessorDistributed("results_vpp");
  _values_ptr = &getVectorPostprocessorValue(
      "results_vpp", getParam<std::string>("results_vector"), !_values_distributed);

  // Sampler
  _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));

  // Resize _coeff, _matrix, _rhs to number of sampler columns
  unsigned int N = _sampler->getNumberOfCols() + 1;
  _coeff.resize(N);
  _matrix.resize(N, N);
  _rhs.resize(N);
}

void
PolynomialRegressionTrainer::initialize()
{
  // Check if results of samples matches number of samples
  __attribute__((unused)) dof_id_type num_rows =
      _values_distributed ? _sampler->getNumberOfLocalRows() : _sampler->getNumberOfRows();
  mooseAssert(num_rows == _values_ptr->size(),
              "Sampler number of rows does not match number of results from vector postprocessor.");

  // Check to make sure there are enough sample points
  if (_sampler->getNumberOfRows() <= _sampler->getNumberOfCols())
    mooseError("Number of sampler rows must be greater than number of columns.");
}

void
PolynomialRegressionTrainer::execute()
{
  // Offset for replicated/distributed result data
  dof_id_type offset = _values_distributed ? _sampler->getLocalRowBegin() : 0;

  // Loop over samples
  for (dof_id_type p = _sampler->getLocalRowBegin(); p < _sampler->getLocalRowEnd(); ++p)
  {
    std::vector<Real> data = _sampler->getNextLocalRow();
    data.insert(data.begin(), 1.0);

    for (unsigned int i = 0; i < data.size(); ++i)
    {
      for (unsigned int j = 0; j < data.size(); ++j)
        _matrix(i, j) += data[i] * data[j];
      _rhs(i) += data[i] * (*_values_ptr)[p - offset];
    }
  }
}

void
PolynomialRegressionTrainer::finalize()
{
  gatherSum(_matrix.get_values());
  gatherSum(_rhs.get_values());

  DenseVector<Real> sol;
  _matrix.lu_solve(_rhs, sol);
  _coeff = sol.get_values();
}
