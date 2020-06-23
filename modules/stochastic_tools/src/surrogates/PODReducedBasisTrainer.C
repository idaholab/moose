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
  params.addRequiredParam<std::vector<std::string>>("var_names", "Names of variables we want to extract from solution vectors.");
  params.addRequiredParam<std::vector<std::string>>("tag_names", "Names of tags for the reduced operators.");

  return params;
}

PODReducedBasisTrainer::PODReducedBasisTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
  _var_names(getParam<std::vector<std::string>>("var_names")),
  _tag_names(getParam<std::vector<std::string>>("tag_names")),
  _base_completed(false)
{
}

void
PODReducedBasisTrainer::initialSetup()
{
  _snapshots.clear();
  _snapshots.resize(_var_names.size());

  _base.clear();
  _base.resize(_var_names.size());

  _corr_mx.clear();
  _corr_mx.resize(_var_names.size());

  _eigenvalues.clear();
  _eigenvalues.resize(_var_names.size());

  _eigenvectors.clear();
  _eigenvectors.resize(_var_names.size());

}

void
PODReducedBasisTrainer::initialize()
{
}

void
PODReducedBasisTrainer::execute()
{
  if (!_base_completed)
  {
    computeCorrelationMatrix();

    computeEigenDecomposition();

    computeBasisVectors();

    _base_completed = true;
  }

  printReducedOperators();

}

void
PODReducedBasisTrainer::finalize()
{
}

void
PODReducedBasisTrainer::addToReducedOperator(unsigned int base_i, unsigned int tag_i, std::vector<DenseVector<Real>>& residual)
{
  if (residual.size() != _base.size())
    mooseError("The number of residual blocks is not equal to the number of variables!");

  unsigned int counter = 0;
  for (unsigned int var_i=0; var_i<_var_names.size(); ++var_i)
  {
    if (residual[var_i].size() != _base[var_i][0].size())
      mooseError("The size of the component residual is not the same as the size of the basis vector!",
                 residual[var_i].size()," != ",_base[var_i][0].size());

    for (unsigned int base_j=0; base_j<_base[var_i].size(); ++base_j)
    {
      _red_operators[tag_i](counter, base_i) = residual[var_i].dot(_base[var_i][base_j]);
      counter++;
    }
  }
}

void
PODReducedBasisTrainer::addSnapshot(unsigned int v_ind, DenseVector<Real>& snapshot)
{
  _snapshots[v_ind].push_back(snapshot);
}

unsigned int
PODReducedBasisTrainer::getSumBaseSize()
{
  unsigned int sum = 0;
  for (unsigned int i=0; i<_base.size(); ++i)
  {
    sum += _base[i].size();
  }
  return sum;
}

void
PODReducedBasisTrainer::computeCorrelationMatrix()
{
  for (unsigned int v_ind=0; v_ind<_snapshots.size(); ++v_ind)
  {
    unsigned int no_snaps = _snapshots[v_ind].size();

    _corr_mx[v_ind] = DenseMatrix<Real>(no_snaps, no_snaps);

    for (unsigned int j=0; j<no_snaps; ++j)
    {
      for (unsigned int k=0; k<no_snaps; ++k)
      {
        if (j>=k)
          _corr_mx[v_ind](j,k)=_snapshots[v_ind][j].dot(_snapshots[v_ind][k]);
      }
    }

    for (unsigned int j=0; j<no_snaps; ++j)
    {
      for (unsigned int k=0; k<no_snaps; ++k)
      {
        if (j<k)
          _corr_mx[v_ind](j,k) = _corr_mx[v_ind](k,j);
      }
    }
  }
}

void
PODReducedBasisTrainer::computeEigenDecomposition()
{
  for (unsigned int v_ind=0; v_ind<_corr_mx.size(); ++v_ind)
  {
    unsigned int no_snaps = _corr_mx[v_ind].n();

    DenseVector<Real> eigenvalues(no_snaps);
    DenseMatrix<Real> eigenvectors(no_snaps, no_snaps);

    //Creating a temporary placeholder for the imaginary parts of the eigenvalues
    DenseVector<Real> eigenvalues_imag(no_snaps);

    // Performing the eigenvalue decomposition
    _corr_mx[v_ind].evd_left(eigenvalues, eigenvalues_imag, eigenvectors);

    // Sorting the eigenvectors and eigenvalues based on the magnitude of
    // the eigenvalues
    std::vector<unsigned int> idx(eigenvalues.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::vector<Real>& v = eigenvalues.get_values();

    std::stable_sort(idx.begin(),
                     idx.end(),
                     [&v](unsigned int i, unsigned int j){return v[i]>v[j];});

    _eigenvalues[v_ind] = DenseVector<Real>(eigenvalues.size());
    _eigenvectors[v_ind] = DenseMatrix<Real>(eigenvalues.size(), eigenvalues.size());

    for (unsigned int j=0; j<_eigenvalues[v_ind].size(); ++j)
    {
      _eigenvalues[v_ind](j) = eigenvalues(idx[j]);
      for (unsigned int k=0; k<_eigenvalues[v_ind].size(); ++k)
      {
        _eigenvectors[v_ind](k,j) = eigenvectors(k,idx[j]);
      }
    }
  }
}

void
PODReducedBasisTrainer::computeBasisVectors()
{
  for (unsigned int v_ind=0; v_ind<_eigenvectors.size(); ++v_ind)
  {
    unsigned int no_bases = _eigenvectors[v_ind].n();

    _base[v_ind] = std::vector<DenseVector<Real>>(no_bases);

    for (unsigned int j=0; j<no_bases; ++j)
    {
      _base[v_ind][j].resize(_snapshots[v_ind][0].size());

      for (unsigned int k=0; k<_snapshots[v_ind].size(); ++k)
      {
        DenseVector<Real> tmp(_snapshots[v_ind][k]);
        tmp.scale(_eigenvectors[v_ind](k,j));

        _base[v_ind][j] += tmp;
      }
      _base[v_ind][j] *= (1.0/sqrt(_eigenvalues[v_ind](j)));
    }
  }
}

void
PODReducedBasisTrainer::initReducedOperators()
{
  _red_operators.resize(_tag_names.size());
  unsigned int base_num = getSumBaseSize();
  for(unsigned int op_i=0; op_i<_red_operators.size(); ++op_i)
  {
    _red_operators[op_i] = DenseMatrix<Real>(base_num, base_num);
  }
}

void
PODReducedBasisTrainer::printReducedOperators()
{
  for(unsigned int op_i=0; op_i<_red_operators.size(); ++op_i)
  {
    std::cout << _red_operators[op_i] << std::endl;
  }
}
