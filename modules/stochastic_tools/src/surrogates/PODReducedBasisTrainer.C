//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PODReducedBasisTrainer.h"

registerMooseObject("StochasticToolsApp", PODReducedBasisTrainer);

InputParameters
PODReducedBasisTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();

  params.addClassDescription("Computes the reduced subspace plus the reduced operators for "
                             "POD-RB surrogate.");
  params.addRequiredParam<MultiAppName>("trainer_name", "Trainer object that contains the solutions for different samples.");

  return params;
}

PODReducedBasisTrainer::PODReducedBasisTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters)
{
}

void
PODReducedBasisTrainer::initialSetup()
{
}

void
PODReducedBasisTrainer::initialize()
{
}

void
PODReducedBasisTrainer::execute()
{
  _no_snaps = _solution_vectors.size();
  _corr_mx.resize(_no_snaps, _no_snaps);
  _eigenvalues.resize(_no_snaps);
  _eigenvectors.resize(_no_snaps, _no_snaps);

  computeCorrelationMatrix();

  computeEigenDecomposition();

  computeBasisVectors();

}

void
PODReducedBasisTrainer::finalize()
{
  // To make sure that the pointers go out of scope before the multiapp object
    _solution_vectors.clear();
}

void
PODReducedBasisTrainer::addSnapshot(std::unique_ptr<NumericVector<Number>> new_vector)
{
  _solution_vectors.push_back(std::move(new_vector));
}

void
PODReducedBasisTrainer::computeCorrelationMatrix()
{
  //_communicator.allgather(_solution_vectors);

  // Computing the correlation matrix and using the fact that it is
  // symmetric
  for (unsigned int i=0; i<_no_snaps; ++i)
  {
    for (unsigned int j=0; j<_no_snaps; ++j)
    {
      if (i>=j)
        _corr_mx(i,j)=_solution_vectors[i]->dot(*_solution_vectors[j]);
    }
  }

  for (unsigned int i=0; i<_no_snaps; ++i)
  {
    for (unsigned int j=0; j<_no_snaps; ++j)
    {
      if (i<j)
        _corr_mx(i,j) = _corr_mx(j,i);
    }
  }
}

void
PODReducedBasisTrainer::computeEigenDecomposition()
{
  //Creating a temporary placeholder for the imaginary parts of the eigenvalues
  DenseVector<Real> eigenvalues_imag(_no_snaps);

  // Performing the eigenvalue decomposition
  _corr_mx.evd_left(_eigenvalues, eigenvalues_imag, _eigenvectors);

  // Sorting the eigenvectors and eigenvalues based on the magnitude of
  // the eigenvalues
  std::vector<unsigned int> idx(_eigenvalues.size());
  std::iota(idx.begin(), idx.end(), 0);
  std::vector<Real>& v = _eigenvalues.get_values();

  std::stable_sort(idx.begin(),
                   idx.end(),
                   [&v](unsigned int i, unsigned int j){return v[i]>v[j];});

  DenseVector<Real> tmpVector(_eigenvalues.size());
  DenseMatrix<Real> tmpMatrix(_eigenvalues.size(), _eigenvalues.size());
  for (unsigned int i=0; i<_eigenvalues.size(); ++i)
  {
    tmpVector(i)=_eigenvalues(idx[i]);
    for (unsigned int j=0; j<_eigenvalues.size(); ++j)
    {
      tmpMatrix(j,i) = _eigenvectors(j,idx[i]);
    }
  }

  _eigenvalues = tmpVector;
  _eigenvectors = tmpMatrix;
}

void
PODReducedBasisTrainer::computeBasisVectors()
{
  _base.resize(_no_snaps);
  for (unsigned int i=0; i<_base.size(); ++i)
  {
    _base[i]=_solution_vectors[i]->clone();
    _base[i]->zero();
    for (unsigned int j=0; j<_solution_vectors.size(); ++j)
    {
      _base[i]->add(_eigenvectors(j,i), *_solution_vectors[j]);
    }
    _base[i]->scale(1.0/sqrt(_eigenvalues(i)));
  }
}
