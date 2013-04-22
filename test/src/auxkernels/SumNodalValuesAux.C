/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SumNodalValuesAux.h"

template<>
InputParameters validParams<SumNodalValuesAux>()
{
  InputParameters params = validParams<AuxNodalScalarKernel>();
  params.addRequiredCoupledVar("sum_var", "Variable to be summed");

  return params;
}

SumNodalValuesAux::SumNodalValuesAux(const std::string & name, InputParameters parameters) :
    AuxNodalScalarKernel(name, parameters),
    _sum_var(coupledValue("sum_var"))
{
}

SumNodalValuesAux::~SumNodalValuesAux()
{
}

void
SumNodalValuesAux::compute()
{
  _subproblem.reinitNodes(_node_ids, _tid);        // compute variables at nodes
  for (_i = 0; _i < _var.order(); ++_i)
  {
    Real value = computeValue();
    libMesh::Parallel::sum(value);
    _var.setValue(_i, value);                  // update variable data, which is referenced by other kernels, so the value is up-to-date
  }
}

Real
SumNodalValuesAux::computeValue()
{
  Real sum = 0;
  for (unsigned int i = 0; i < _sum_var.size(); i++)
    sum += _sum_var[i];
  return sum;
}
