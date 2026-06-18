//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReducedPressurePIMPLE.h"

using namespace libMesh;

registerMooseObject("NavierStokesApp", ReducedPressurePIMPLE);

InputParameters
ReducedPressurePIMPLE::validParams()
{
  InputParameters params = PIMPLE::validParams();
  params += ReducedPressurePIMPLESolve::validParams();
  params.addClassDescription(
      "Solves the transient Navier-Stokes equations using a reduced-pressure PIMPLE algorithm with "
      "sharp-interface face-flux hooks and linear finite volume variables.");
  return params;
}

ReducedPressurePIMPLE::ReducedPressurePIMPLE(const InputParameters & parameters)
  : PIMPLE(parameters), _reduced_pimple_solve(*this)
{
  _fixed_point_solve->setInnerSolve(_reduced_pimple_solve);
}

void
ReducedPressurePIMPLE::postTakeStep()
{
  _reduced_pimple_solve.commitAcceptedTimestepTransportHistory();
}
