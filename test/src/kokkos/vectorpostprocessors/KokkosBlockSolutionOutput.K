//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosBlockSolutionOutput.h"

registerKokkosUserObject("MooseTestApp", KokkosBlockSolutionOutput);

InputParameters
KokkosBlockSolutionOutput::validParams()
{
  InputParameters params = ElementVectorPostprocessor::validParams();
  params.addCoupledVar("variable", "Solution variable");
  return params;
}

KokkosBlockSolutionOutput::KokkosBlockSolutionOutput(const InputParameters & parameters)
  : ElementVectorPostprocessor(parameters),
    _mesh(_fe_problem.mesh().getMesh()),
    _var(kokkosCoupledValue("variable")),
    _block_id_vec(declareVector("block_id")),
    _solution_vec(declareVector("solution"))
{
  _block_id_vec.resize(_mesh.n_subdomains());
  _solution_vec.resize(_mesh.n_subdomains());

  _solution.create(_mesh.n_elem());
}

void
KokkosBlockSolutionOutput::initialize()
{
  std::fill(_solution_vec.begin(), _solution_vec.end(), 0);

  _solution = 0;
}

void
KokkosBlockSolutionOutput::finalize()
{
  _solution.copyToHost();

  std::map<SubdomainID, Real> sum;

  for (auto block : blockIDs())
  {
    sum[block] = 0;

    for (auto elem : _mesh.active_local_subdomain_elements_ptr_range(block))
      sum[block] += _solution[kokkosMesh().getContiguousElementID(elem)];
  }

  dof_id_type i = 0;

  for (auto & [block, value] : sum)
  {
    _block_id_vec[i] = block;
    _solution_vec[i++] = value;
  }

  comm().sum(_solution_vec);
}
