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
  params.addRequiredParam<std::vector<std::string>>("var_names",
                                                    "Names of variables we want to"
                                                    "extract from solution vectors.");
  params.addRequiredParam<std::vector<Real>>("en_limits",
                                             "List of energy retention limits for each variable.");
  params.addRequiredParam<std::vector<std::string>>("tag_names",
                                                    "Names of tags for the reduced operators.");
  params.addParam<std::vector<std::string>>("dir_tag_names",
                                            std::vector<std::string>(0),
                                            "Names of tags for reduced operators corresponding to dirichlet BCs.");
  params.addRequiredParam<std::vector<unsigned int>>("independent",
                                                     "List of bools describing if the tags"
                                                     " correspond to and independent operator or not.");
  return params;
}

PODReducedBasisTrainer::PODReducedBasisTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
  _var_names(declareModelData<std::vector<std::string>>("_var_names", getParam<std::vector<std::string>>("var_names"))),
  _en_limits(getParam<std::vector<Real>>("en_limits")),
  _tag_names(declareModelData<std::vector<std::string>>("_tag_names", getParam<std::vector<std::string>>("tag_names"))),
  _dir_tag_names(declareModelData<std::vector<std::string>>("_dir_tag_names", getParam<std::vector<std::string>>("dir_tag_names"))),
  _independent(declareModelData<std::vector<unsigned int>>("_independent", getParam<std::vector<unsigned int>>("independent"))),
  _base(declareModelData<std::vector<std::vector<DenseVector<Real>>>>("_base")),
  _red_operators(declareModelData<std::vector<DenseMatrix<Real>>>("_red_operators")),
  _base_completed(false)
{
  if (_en_limits.size() != _var_names.size())
      paramError("en_limits",
                 "The number of elements is not equal to the number"
                 " of elements in 'var_names'");

  if (_tag_names.size() != _independent.size())
    paramError("independent",
               "The number of elements is not equal to the number"
               " of elements in 'tag_names'");
}

void
PODReducedBasisTrainer::initialSetup()
{
  // Initializing the containers for the essential data to construct the
  // reduced operators.
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
  // If the base is not ready yet, create it by performing the POD on the
  // snapshot matrices.
  if (!_base_completed)
  {
    computeCorrelationMatrix();
    computeEigenDecomposition();
    computeBasisVectors();
    _base_completed = true;
  }
}

void
PODReducedBasisTrainer::finalize()
{
}

void
PODReducedBasisTrainer::addSnapshot(unsigned int v_ind, DenseVector<Real>& snapshot)
{
  _snapshots[v_ind].push_back(snapshot);
}

void
PODReducedBasisTrainer::computeCorrelationMatrix()
{
  // Looping over all the variables.
  for (unsigned int v_ind=0; v_ind<_snapshots.size(); ++v_ind)
  {
    unsigned int no_snaps = _snapshots[v_ind].size();

    // Initializing the correlation matrix.
    _corr_mx[v_ind] = DenseMatrix<Real>(no_snaps, no_snaps);

    // Filling the correlation matrix with the inner products between snapshots
    // and utilizing the fact the the correlation matrix is symmetric.
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

    // Initializing temprary objects for the eigenvalues and eigenvectors since
    // evd_left() returns an unordered vector of eigenvalues.
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

    // Getting the indices to be able to copy the corresponding eigenvector too.
    std::stable_sort(idx.begin(),
                     idx.end(),
                     [&v](unsigned int i, unsigned int j){return v[i]>v[j];});

    // Getting a cutoff for the number of modes. The functio nrequires a sorted list,
    // thus the temporary vector is sorted.
    std::stable_sort(v.begin(), v.end(), std::greater<Real>());
    unsigned int cutoff = determineNumberOfModes(_en_limits[v_ind], v);

    // Initializing the actual containers for the eigenvectors and eigenvalues.
    _eigenvalues[v_ind] = DenseVector<Real>(cutoff);
    _eigenvectors[v_ind] = DenseMatrix<Real>(eigenvectors.m(), cutoff);

    // Copying the kept eigenvalues and eigenvectors in a sorted container.
    for (unsigned int j=0; j<cutoff; ++j)
    {
      _eigenvalues[v_ind](j) = eigenvalues(j);
      for (unsigned int k=0; k<_eigenvectors[v_ind].m(); ++k)
      {
        _eigenvectors[v_ind](k,j) = eigenvectors(k,idx[j]);
      }
    }
  }
}

unsigned int
PODReducedBasisTrainer::determineNumberOfModes(Real limit, std::vector<Real>& inp_vec)
{
  Real sum = std::accumulate(inp_vec.begin(), inp_vec.end(), 0.0);

  Real part_sum = 0.0;
  for(unsigned int i=0; i<inp_vec.size(); ++i)
  {
    part_sum += inp_vec[i];
    if (part_sum/sum > limit)
      return(i+1);
  }
  return(inp_vec.size());
}

void
PODReducedBasisTrainer::computeBasisVectors()
{
  // Looping over all the variables.
  for (unsigned int v_ind=0; v_ind<_eigenvectors.size(); ++v_ind)
  {
    unsigned int no_bases = _eigenvalues[v_ind].size();

    // Initializing the containers for the basis vectors.
    _base[v_ind] = std::vector<DenseVector<Real>>(no_bases);

    // Filling the containers using the snapshots and the eigenvalues and
    // eigenvectors of the correlation matrices.
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
  if(!_base_completed)
    mooseError("There are no basis vectors available, call computeBasisVectors() first!");

  _red_operators.resize(_tag_names.size());

  // Getting the sum of the ranks of the different bases. Every reduced operator
  // is resized with this number. This way the construction can be more general.
  unsigned int base_num = getSumBaseSize();

  // Initializing each operator (each operator with a tag).
  for(unsigned int tag_i=0; tag_i<_red_operators.size(); ++tag_i)
  {
    if(_independent[tag_i])
      _red_operators[tag_i].resize(base_num, 1);
    else
      _red_operators[tag_i].resize(base_num, base_num);
  }
}

void
PODReducedBasisTrainer::addToReducedOperator(unsigned int base_i,
                                             unsigned int tag_i,
                                             std::vector<DenseVector<Real>>& residual)
{
  // Checking if the reduced operators has been initialized.
  if (_red_operators[tag_i].n() < base_i+1)
    mooseError("The size of the reduced operator for tag '",
               _tag_names[tag_i],
               "' is not consistent with the input! Call initReducedOperators()",
               " first!");

  // Checking if the residual is consistent with the bases.
  if (residual.size() != _base.size())
    mooseError("The number of residual blocks is not equal to the number of variables!");

  // Computing the elements of the reduced operator using Galerkin projection
  // on the residual.
  unsigned int counter = 0;
  for (unsigned int var_i=0; var_i<_var_names.size(); ++var_i)
  {
    // Checking if the vector sizes are the same.
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

unsigned int
PODReducedBasisTrainer::getSumBaseSize()
{
  unsigned int sum = 0;

  for (unsigned int i=0; i<_base.size(); ++i)
    sum += _base[i].size();

  return sum;
}

void
PODReducedBasisTrainer::printReducedOperators()
{
  for(unsigned int op_i=0; op_i<_red_operators.size(); ++op_i)
  {
    std::cout << _tag_names[op_i] << std::endl << std::endl;
    std::cout << _red_operators[op_i] << std::endl;
  }
}
