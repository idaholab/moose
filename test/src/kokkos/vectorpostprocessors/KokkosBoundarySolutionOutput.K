//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosBoundarySolutionOutput.h"

registerKokkosUserObject("MooseTestApp", KokkosBoundarySolutionOutput);

InputParameters
KokkosBoundarySolutionOutput::validParams()
{
  InputParameters params = SideVectorPostprocessor::validParams();
  params.addCoupledVar("variable", "Solution variable");
  return params;
}

KokkosBoundarySolutionOutput::KokkosBoundarySolutionOutput(const InputParameters & parameters)
  : SideVectorPostprocessor(parameters),
    _mesh(_fe_problem.mesh()),
    _var(kokkosCoupledValue("variable")),
    _boundary_id_vec(declareVector("boundary_id")),
    _solution_vec(declareVector("solution"))
{
  _boundary_id_vec.resize(_mesh.meshBoundaryIds().size());
  _solution_vec.resize(_mesh.meshBoundaryIds().size());

  _solution.create(_mesh.getMaxSidesPerElem(), _mesh.getMesh().n_elem());
}

void
KokkosBoundarySolutionOutput::initialize()
{
  std::fill(_solution_vec.begin(), _solution_vec.end(), 0);

  _solution = 0;
}

void
KokkosBoundarySolutionOutput::finalize()
{
  _solution.copyToHost();

  std::map<BoundaryID, Real> sum;

  for (auto boundary : boundaryIDs())
  {
    sum[boundary] = 0;

    for (auto elem_id : _mesh.getBoundaryActiveSemiLocalElemIds(boundary))
    {
      const auto elem = _mesh.elemPtr(elem_id);

      if (elem->processor_id() == processor_id())
        sum[boundary] += _solution(_mesh.sideWithBoundaryID(elem, boundary),
                                   kokkosMesh().getContiguousElementID(elem));
    }
  }

  dof_id_type i = 0;

  for (auto & [boundary, value] : sum)
  {
    _boundary_id_vec[i] = boundary;
    _solution_vec[i++] = value;
  }

  comm().sum(_solution_vec);
}
