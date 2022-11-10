/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "NoSolveProblem.h"

registerMooseObject("SubChannelApp", NoSolveProblem);

InputParameters
NoSolveProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  return params;
}

NoSolveProblem::NoSolveProblem(const InputParameters & params) : ExternalProblem(params) {}

void
NoSolveProblem::externalSolve()
{
}

void NoSolveProblem::syncSolutions(Direction /*direction*/) {}

bool
NoSolveProblem::converged()
{
  return true;
}
