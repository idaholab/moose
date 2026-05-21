//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadraticPredictor.h"
#include "NonlinearSystem.h"

#include "libmesh/numeric_vector.h"

registerMooseObject("MooseApp", QuadraticPredictor);

InputParameters
QuadraticPredictor::validParams()
{
  InputParameters params = Predictor::validParams();
  params.addClassDescription(
      "Quadratic Lagrange extrapolation predictor based on three accepted solutions.");
  return params;
}

QuadraticPredictor::QuadraticPredictor(const InputParameters & parameters)
  : Predictor(parameters),
    _older_solution(_nl.addVector("quadratic_predictor_older_solution", true, libMesh::GHOSTED)),
    _oldest_solution(_nl.addVector("quadratic_predictor_oldest_solution", true, libMesh::GHOSTED)),
    _dt_older(declareRestartableData<Real>("dt_older", 0)),
    _dt_storage(declareRestartableData<Real>("dt_storage", 0)),
    _history_size(declareRestartableData<unsigned int>("history_size", 0))
{
}

void
QuadraticPredictor::timestepSetup()
{
  Predictor::timestepSetup();

  if (_is_repeated_timestep)
    return;

  // On a new accepted step, sln will already enter apply() as u^n. Therefore this object only
  // stores the two older states needed by the quadratic formula: u^{n-1} and u^{n-2}.
  _older_solution.localize(_oldest_solution);
  _solution_older.localize(_older_solution);

  // _dt_old is h1 = t_n - t_{n-1}; _dt_older is h2 = t_{n-1} - t_{n-2}.
  _dt_older = _dt_storage;
  _dt_storage = _dt_old;

  if (_history_size < 3)
    ++_history_size;
}

bool
QuadraticPredictor::shouldApply()
{
  if (!Predictor::shouldApply())
    return false;

  if (_dt <= 0 || _dt_old <= 0 || _dt_older <= 0 || _history_size < 3)
    return false;

  return true;
}

void
QuadraticPredictor::apply(NumericVector<Number> & sln)
{
  if (_scale == 0.0)
  {
    _console << "  Skipping quadratic predictor because scale factor = 0" << std::endl;
    return;
  }

  _console << "  Applying quadratic predictor with scale factor = " << _scale << std::endl;

  const Real h0 = _dt;
  const Real h1 = _dt_old;
  const Real h2 = _dt_older;

  // Lagrange extrapolation from times t_n, t_{n-1}, and t_{n-2} to t_{n+1}.
  // For uniform time steps these coefficients reduce to (3, -3, 1).
  const Real a0 = (h0 + h1) * (h0 + h1 + h2) / (h1 * (h1 + h2));
  const Real a1 = -h0 * (h0 + h1 + h2) / (h1 * h2);
  const Real a2 = h0 * (h0 + h1) / (h2 * (h1 + h2));

  // sln enters apply() as u^n. Rewriting in place yields the damped predictor
  // u^n + scale * (u_pred_quadratic - u^n) without extra temporary vectors.
  sln *= 1.0 + _scale * (a0 - 1.0);
  sln.add(_scale * a1, _older_solution);
  sln.add(_scale * a2, _oldest_solution);
  sln.close();
}
