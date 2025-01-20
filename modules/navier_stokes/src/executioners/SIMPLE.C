//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SIMPLE.h"
#include "FEProblem.h"

using namespace libMesh;

registerMooseObject("NavierStokesApp", SIMPLE);

InputParameters
SIMPLE::validParams()
{
  InputParameters params = SteadyBase::validParams();
  params.addClassDescription("Solves the Navier-Stokes equations using the SIMPLE algorithm and "
                             "linear finite volume variables.");
  params += SIMPLESolve::validParams();

  return params;
}

SIMPLE::SIMPLE(const InputParameters & parameters) : SteadyBase(parameters), _simple_solve(*this)
{
  _fixed_point_solve->setInnerSolve(_simple_solve);
}

void
SIMPLE::init()
{
  _problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _problem.initialSetup();
  _simple_solve.checkIntegrity();
  _simple_solve.linkRhieChowUserObject();
  _simple_solve.setupPressurePin();
}
