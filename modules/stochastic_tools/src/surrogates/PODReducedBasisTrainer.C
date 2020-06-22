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
  params.addRequiredParam<std::vector<std::string>>("var_names", "Trainer object that contains the solutions for different samples.");

  return params;
}

PODReducedBasisTrainer::PODReducedBasisTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
  _var_names(getParam<std::vector<std::string>>("var_names"))
{
}

void
PODReducedBasisTrainer::initialSetup()
{
  _snapshots.clear();
  for (auto var_name : _var_names)
  {
    _snapshots[var_name] =  std::vector<DenseVector<Real>>(0);
  }
}

void
PODReducedBasisTrainer::initialize()
{
}

void
PODReducedBasisTrainer::execute()
{

  computeCorrelationMatrix();

  computeEigenDecomposition();

  computeBasisVectors();

}

void
PODReducedBasisTrainer::finalize()
{
}

void
PODReducedBasisTrainer::addSnapshot(std::string var_name, DenseVector<Real>& snapshot)
{
  _snapshots[var_name].push_back(snapshot);
}

unsigned int
PODReducedBasisTrainer::getSumBaseSize()
{
  unsigned int sum = 0;
  for (auto it : _base)
  {
    sum += it.second.size();
  }
  return sum;
}

void
PODReducedBasisTrainer::computeCorrelationMatrix()
{
  for (auto it : _snapshots)
  {
    const std::string& var_name = it.first;
    unsigned int no_snaps = it.second.size();

    _corr_mx[var_name] = DenseMatrix<Real>(no_snaps, no_snaps);

    for (unsigned int j=0; j<no_snaps; ++j)
    {
      for (unsigned int k=0; k<no_snaps; ++k)
      {
        if (j>=k)
          _corr_mx[var_name](j,k)=it.second[j].dot(it.second[k]);
      }
    }

    for (unsigned int j=0; j<no_snaps; ++j)
    {
      for (unsigned int k=0; k<no_snaps; ++k)
      {
        if (j<k)
          _corr_mx[var_name](j,k) = _corr_mx[var_name](k,j);
      }
    }
  }
}

void
PODReducedBasisTrainer::computeEigenDecomposition()
{
  for (auto it : _corr_mx)
  {
    const std::string& var_name = it.first;
    unsigned int no_snaps = it.second.n();

    DenseVector<Real> eigenvalues(no_snaps);
    DenseMatrix<Real> eigenvectors(no_snaps, no_snaps);

    //Creating a temporary placeholder for the imaginary parts of the eigenvalues
    DenseVector<Real> eigenvalues_imag(no_snaps);

    // Performing the eigenvalue decomposition
    it.second.evd_left(eigenvalues, eigenvalues_imag, eigenvectors);

    // Sorting the eigenvectors and eigenvalues based on the magnitude of
    // the eigenvalues
    std::vector<unsigned int> idx(eigenvalues.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::vector<Real>& v = eigenvalues.get_values();

    std::stable_sort(idx.begin(),
                     idx.end(),
                     [&v](unsigned int i, unsigned int j){return v[i]>v[j];});

    _eigenvalues[var_name] = DenseVector<Real>(eigenvalues.size());
    _eigenvectors[var_name] = DenseMatrix<Real>(eigenvalues.size(), eigenvalues.size());

    for (unsigned int j=0; j<_eigenvalues[var_name].size(); ++j)
    {
      _eigenvalues[var_name](j) = eigenvalues(idx[j]);
      for (unsigned int k=0; k<_eigenvalues[var_name].size(); ++k)
      {
        _eigenvectors[var_name](k,j) = eigenvectors(k,idx[j]);
      }
    }
  }
}

void
PODReducedBasisTrainer::computeBasisVectors()
{
  for (auto it : _eigenvectors)
  {
    const std::string& var_name = it.first;
    unsigned int no_bases = it.second.n();

    _base[var_name] = std::vector<DenseVector<Real>>(no_bases);

    for (unsigned int j=0; j<no_bases; ++j)
    {
      _base[var_name][j].resize(_snapshots[var_name][0].size());

      for (unsigned int k=0; k<_snapshots[var_name].size(); ++k)
      {
        DenseVector<Real> tmp(_snapshots[var_name][k]);
        tmp.scale(_eigenvectors[var_name](k,j));

        _base[var_name][j] += tmp;
      }
      _base[var_name][j] *= (1.0/sqrt(_eigenvalues[var_name](j)));
    }
  }
}
