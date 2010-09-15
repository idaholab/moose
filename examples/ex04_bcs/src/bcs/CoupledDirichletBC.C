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

#include "CoupledDirichletBC.h"

template<>
InputParameters validParams<CoupledDirichletBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  
  // Here we are adding a parameter that will be extracted from the input file by the Parser
  params.addParam<Real>("value", 0.0, "Value multiplied by the coupled value on the boundary");
  params.addRequiredCoupledVar("some_var", "Value on the Boundary");
  params.set<bool>("_integrated") = false;
  return params;
}

CoupledDirichletBC::CoupledDirichletBC(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters),
   
   /**
    * Grab the parameter for the multiplier.
    */
   _value(getParam<Real>("value")),

   /**
    * Get a reference to the coupled variable's values.
    */
   _some_var_val(coupledValue("some_var"))
{}

Real
CoupledDirichletBC::computeQpResidual()
{
  return _u[_qp]-(_value*_some_var_val[_qp]);
}
