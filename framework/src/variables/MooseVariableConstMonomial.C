//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableConstMonomial.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "Assembly.h"

#include "libmesh/quadrature.h"

MooseVariableConstMonomial::MooseVariableConstMonomial(const InputParameters & parameters)
  : MooseVariable(parameters)
{
}

void
MooseVariableConstMonomial::computeElemValues()
{
  _element_data->setGeometry(Moose::Volume);

  // We have not optimized AD calculations for const monomials yet, so we fall back on the
  // non-optimized routine
  if (_element_data->needsAD() && _subproblem.currentlyComputingJacobian())
    _element_data->computeValues();
  else
    _element_data->computeMonomialValues();
}

void
MooseVariableConstMonomial::computeElemValuesFace()
{
  _element_data->setGeometry(Moose::Face);

  // We have not optimized AD calculations for const monomials yet, so we fall back on the
  // non-optimized routine
  if (_element_data->needsAD() && _subproblem.currentlyComputingJacobian())
    _element_data->computeValues();
  else
    _element_data->computeMonomialValues();
}

void
MooseVariableConstMonomial::computeNeighborValues()
{
  _neighbor_data->setGeometry(Moose::Volume);

  // We have not optimized AD calculations for const monomials yet, so we fall back on the
  // non-optimized routine
  if (_neighbor_data->needsAD() && _subproblem.currentlyComputingJacobian())
    _neighbor_data->computeValues();
  else
    _neighbor_data->computeMonomialValues();
}

void
MooseVariableConstMonomial::computeNeighborValuesFace()
{
  _neighbor_data->setGeometry(Moose::Face);

  // We have not optimized AD calculations for const monomials yet, so we fall back on the
  // non-optimized routine
  if (_neighbor_data->needsAD() && _subproblem.currentlyComputingJacobian())
    _neighbor_data->computeValues();
  else
    _neighbor_data->computeMonomialValues();
}
