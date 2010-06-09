#include "CoupledDirichletBC.h"

template<>
InputParameters validParams<CoupledDirichletBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addParam<Real>("value", 0.0, "Value multiplied by the coupled value on the boundary");
  return params;
}

CoupledDirichletBC::CoupledDirichletBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, setIntegratedParam(parameters, false)),
   
   /**
    * Grab the parameter for the multiplier.
    */
   _value(_parameters.get<Real>("value")),

   /**
    * Get a reference to the coupled variable's values.
    */
   _some_var_val(coupledVal("some_var"))
{}

Real
CoupledDirichletBC::computeQpResidual()
{
  return _u[_qp]-(_value*_some_var_val[_qp]);
}
