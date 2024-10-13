//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdamsPredictor.h"
#include "NonlinearSystem.h"

#include "libmesh/numeric_vector.h"

registerMooseObject("MooseApp", AdamsPredictor);

InputParameters
AdamsPredictor::validParams()
{
  InputParameters params = Predictor::validParams();
  params.addClassDescription(
      "Implements an explicit Adams predictor based on two old solution vectors.");
  params.addParam<int>("order", 2, "The maximum reachable order of the Adams-Bashforth Predictor");
  return params;
}

AdamsPredictor::AdamsPredictor(const InputParameters & parameters)
  : Predictor(parameters),
    _order(getParam<int>("order")),
    _current_old_solution(_nl.addVector("AB2_current_old_solution", true, libMesh::GHOSTED)),
    _older_solution(_nl.addVector("AB2_older_solution", true, libMesh::GHOSTED)),
    _oldest_solution(_nl.addVector("AB2_rejected_solution", true, libMesh::GHOSTED)),
    _tmp_previous_solution(_nl.addVector("tmp_previous_solution", true, libMesh::GHOSTED)),
    _tmp_residual_old(_nl.addVector("tmp_residual_old", true, libMesh::GHOSTED)),
    _tmp_third_vector(_nl.addVector("tmp_third_vector", true, libMesh::GHOSTED)),
    _dt_older(declareRestartableData<Real>("dt_older", 0)),
    _dtstorage(declareRestartableData<Real>("dtstorage", 0))
{
}

void
AdamsPredictor::timestepSetup()
{
  Predictor::timestepSetup();

  // if the time step number hasn't changed (=repeated timestep) then do nothing
  if (_is_repeated_timestep)
    return;

  // Otherwise move back the previous old solution and copy the current old solution,
  // This will probably not work with DT2, but I don't need to get it to work with dt2.
  _older_solution.localize(_oldest_solution);
  // Set older solution to hold the previous old solution
  _current_old_solution.localize(_older_solution);
  // Set current old solution to hold what it says.
  (_nl.solutionOld()).localize(_current_old_solution);
  // Same thing for dt
  _dt_older = _dtstorage;
  _dtstorage = _dt_old;
}

bool
AdamsPredictor::shouldApply()
{
  if (!Predictor::shouldApply())
    return false;

  // AB2 can only be applied if there are enough old solutions
  // AB1 could potentially be used for the time step prior?
  // It would be possible to do VSVO Adams, Kevin has the info
  // Doing so requires a time stack of some sort....
  if (_dt == 0 || _dt_old == 0 || _dt_older == 0 || _t_step < 2)
    return false;
  else
    return true;
}

void
AdamsPredictor::apply(NumericVector<Number> & sln)
{
  // localize current solution to working vec
  sln.localize(_solution_predictor);
  // NumericVector<Number> & vector1 = _tmp_previous_solution;
  NumericVector<Number> & vector2 = _tmp_residual_old;
  NumericVector<Number> & vector3 = _tmp_third_vector;

  Real commonpart = _dt / _dt_old;
  Real firstpart = (1 + .5 * commonpart);
  Real secondpart = .5 * _dt / _dt_older;

  _older_solution.localize(vector2);
  _oldest_solution.localize(vector3);

  _solution_predictor *= 1 + commonpart * firstpart;
  vector2 *= -1. * commonpart * (firstpart + secondpart);
  vector3 *= commonpart * secondpart;

  _solution_predictor += vector2;
  _solution_predictor += vector3;

  _solution_predictor.localize(sln);
}
