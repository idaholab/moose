//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestBoundaryFlux.h"

registerMooseObject("RdgTestApp", TestBoundaryFlux);

InputParameters
TestBoundaryFlux::validParams()
{
  InputParameters params = BoundaryFluxBase::validParams();
  params.addClassDescription("Boundary flux used for testing");
  return params;
}

TestBoundaryFlux::TestBoundaryFlux(const InputParameters & parameters)
  : BoundaryFluxBase(parameters)
{
}

void
TestBoundaryFlux::calcFlux(unsigned int /*iside*/,
                           dof_id_type /*ielem*/,
                           const std::vector<Real> & solution,
                           const RealVectorValue & normal,
                           std::vector<Real> & flux) const
{
  mooseAssert(solution.size() == 3, "Solution vector must have exactly 3 entries.");

  const Real & A = solution[0];
  const Real & B = solution[1];
  const Real & C = solution[2];

  flux.resize(2);
  flux[0] = (A - B) * C * normal(0);
  flux[1] = A * B * normal(0);
}

void
TestBoundaryFlux::calcJacobian(unsigned int /*iside*/,
                               dof_id_type /*ielem*/,
                               const std::vector<Real> & /*uvec*/,
                               const RealVectorValue & /*dwave*/,
                               DenseMatrix<Real> & /*jac1*/) const
{
  mooseError("Not implemented.");
}
