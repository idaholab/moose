//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "PIMPLE.h"
#include "FEProblem.h"

using namespace libMesh;

registerMooseObject("NavierStokesApp", PIMPLE);

InputParameters
PIMPLE::validParams()
{
  InputParameters params = SteadyBase::validParams();
  params.addClassDescription("Solves the transient Navier-Stokes equations using the PIMPLE algorithm and "
                             "linear finite volume variables.");
  params += PIMPLESolve::validParams();

  return params;
}

PIMPLE::PIMPLE(const InputParameters & parameters) : SteadyBase(parameters), _pimple_solve(*this)
{
  _fixed_point_solve->setInnerSolve(_pimple_solve);
}

void
PIMPLE::init()
{
  _problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _problem.initialSetup();
  _pimple_solve.linkRhieChowUserObject();
  _pimple_solve.setupPressurePin();
}
