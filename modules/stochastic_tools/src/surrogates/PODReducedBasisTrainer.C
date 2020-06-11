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

  std::cout << _corr_mx << std::endl;
}

void
PODReducedBasisTrainer::finalize()
{
    // gatherSum(_corr_mx.get_values());
    // std::cout << _corr_mx << std::endl;
}

void
PODReducedBasisTrainer::addSnapshot(NumericVector<Number>& new_vector)
{
  _solution_vectors.push_back(new_vector.clone());
}
