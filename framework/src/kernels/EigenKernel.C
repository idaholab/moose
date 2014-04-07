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

#include "EigenKernel.h"

template<>
InputParameters validParams<EigenKernel>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

EigenKernel::EigenKernel(const std::string & name, InputParameters parameters)
    :Kernel(name,parameters),
     _u_old(valueOld()), // old solution is made available by eigensolvers
     _current(_fe_problem.parameters().set<bool>("eigen_on_current")),
     _eigen_pp(_fe_problem.parameters().set<PostprocessorName>("eigen_postprocessor")),
     _eigen(getPostprocessorValueByName(_eigen_pp)),
     _eigen_old(getPostprocessorValueOldByName(_eigen_pp))
{
}

Real
EigenKernel::computeQpResidual()
{
  if (_current)
    return -_u[_qp] / _eigen * _test[_i][_qp];
  else
    return -_u_old[_qp] / _eigen_old * _test[_i][_qp];
}
