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

  params.addClassDescription("Computes coefficients for polynomial regession model.");

  params.addRequiredParam<SamplerName>("sampler", "Training set defined by a sampler object.");
  params.addRequiredParam<VectorPostprocessorName>(
      "results_vpp", "Vectorpostprocessor with results of samples created by trainer.");
  params.addRequiredParam<std::string>(
      "results_vector",
      "Name of vector from vectorpostprocessor with results of samples created by trainer");
  params.addRequiredParam<unsigned int>("max_degree",
                                        "Maximum polynomial degree to use for the regression.");

  return params;
}

PolynomialRegressionTrainer::PolynomialRegressionTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _coeff(declareModelData<std::vector<Real>>("_coeff")),
    _power_matrix(declareModelData<std::vector<std::vector<unsigned int>>>("_power_matrix")),
    _max_degree(declareModelData<unsigned int>("_max_degree", getParam<unsigned int>("max_degree")))
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

  _n_dims = _sampler->getNumberOfCols();

  // Initializing power matrix, using _max_degree+1 to tackle the indexing offset
  // within generateTuple
  _power_matrix =
      StochasticTools::MultiDimPolynomialGenerator::generateTuple(_n_dims, _max_degree + 1);

  _n_poly_terms = _power_matrix.size();

  // Check if we have enough data points to solve the problem
  if (_sampler->getNumberOfRows() <= _n_poly_terms)
    mooseError("Number of data points must be greater than the number of terms in the polynomial.");

  // Resize _coeff, _matrix, _rhs to number of sampler columns
  _coeff.resize(_n_poly_terms);
  _matrix.resize(_n_poly_terms, _n_poly_terms);
  _rhs.resize(_n_poly_terms);
}

void
PolynomialRegressionTrainer::initialize()
{
  // Check if results of samples matches number of samples
  __attribute__((unused)) dof_id_type num_rows =
      _values_distributed ? _sampler->getNumberOfLocalRows() : _sampler->getNumberOfRows();
  mooseAssert(num_rows == _values_ptr->size(),
              "Sampler number of rows does not match number of results from vector postprocessor.");
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

    for (unsigned int i = 0; i < _n_poly_terms; ++i)
    {
      std::vector<unsigned int> i_powers(_power_matrix[i]);

      Real i_value(1.0);
      for (unsigned int ii = 0; ii < data.size(); ++ii)
        i_value *= pow(data[ii], i_powers[ii]);

      for (unsigned int j = 0; j < _n_poly_terms; ++j)
      {
        std::vector<unsigned int> j_powers(_power_matrix[j]);

        Real j_value(1.0);
        for (unsigned int jj = 0; jj < data.size(); ++jj)
          j_value *= pow(data[jj], j_powers[jj]);

        _matrix(i, j) += i_value * j_value;
      }

      _rhs(i) += i_value * (*_values_ptr)[p - offset];
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
