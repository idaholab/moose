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

#include "ScalarDirichletBC.h"

template<>
InputParameters validParams<ScalarDirichletBC>()
{
  InputParameters params = validParams<NodalBC>();
  // Here we are adding a parameter that will be extracted from the input file by the Parser
  params.addRequiredCoupledVar("scalar_var", "Value of the scalar variable");
  return params;
}

ScalarDirichletBC::ScalarDirichletBC(const std::string & name, InputParameters parameters) :
    NodalBC(name, parameters),

    /**
     * Get a reference to the coupled variable's values.
     */
    _scalar_val(coupledScalarValue("scalar_var"))
{}

Real
ScalarDirichletBC::computeQpResidual()
{
  // We coupled in a first order scalar variable, thus there is only one value in _scalar_val (and it is - big surprise - on index 0)
  return _u[_qp] - _scalar_val[0];
}
