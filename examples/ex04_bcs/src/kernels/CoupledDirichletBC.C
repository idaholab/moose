#include "CoupledDirichletBC.h"

template<>
InputParameters validParams<CoupledDirichletBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  
  // Here we are adding a parameter that will be extracted from the input file by the Parser
  params.addParam<Real>("value", 0.0, "Value multiplied by the coupled value on the boundary");
  params.addRequiredCoupledVar("some_var", "Value on the Boundary");
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
