//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNodalSolutionOutput.h"

registerKokkosUserObject("MooseTestApp", KokkosNodalSolutionOutput);

InputParameters
KokkosNodalSolutionOutput::validParams()
{
  InputParameters params = NodalVectorPostprocessor::validParams();
  params.addCoupledVar("variable", "Solution variable");
  return params;
}

KokkosNodalSolutionOutput::KokkosNodalSolutionOutput(const InputParameters & parameters)
  : NodalVectorPostprocessor(parameters),
    _mesh(_fe_problem.mesh()),
    _var(kokkosCoupledValue("variable")),
    _block_id_vec(declareVector("block_id")),
    _solution_vec(declareVector("solution"))
{
  _block_id_vec.resize(_mesh.nSubdomains());
  _solution_vec.resize(_mesh.nSubdomains());

  _solution.create(_mesh.nNodes());
}

void
KokkosNodalSolutionOutput::initialize()
{
  std::fill(_solution_vec.begin(), _solution_vec.end(), 0);

  _solution = 0;
}

void
KokkosNodalSolutionOutput::finalize()
{
  _solution.copyToHost();

  std::map<SubdomainID, Real> sum;

  for (auto block : blockIDs())
    sum[block] = 0;

  for (auto node : *_mesh.getLocalNodeRange())
    for (auto block : _mesh.getNodeBlockIds(*node))
      sum[block] += _solution[kokkosMesh().getContiguousNodeID(node)];

  dof_id_type i = 0;

  for (auto & [block, value] : sum)
  {
    _block_id_vec[i] = block;
    _solution_vec[i++] = value;
  }

  comm().sum(_solution_vec);
}
