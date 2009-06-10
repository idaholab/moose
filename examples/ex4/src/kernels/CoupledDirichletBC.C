#include "CoupledDirichletBC.h"

template<>
Parameters valid_params<CoupledDirichletBC>()
{
  Parameters params;
  params.set<Real>("value")=0.0;
  return params;
}

CoupledDirichletBC::CoupledDirichletBC(std::string name,
                                       Parameters parameters,
                                       std::string var_name,
                                       unsigned int boundary_id,
                                       std::vector<std::string> coupled_to,
                                       std::vector<std::string> coupled_as)
  :BoundaryCondition(name, parameters, var_name, false, boundary_id, coupled_to, coupled_as),
   
   /**
    * Grab the parameter for the multiplier.
    */
   _value(_parameters.get<Real>("value")),

   /**
    * Get a reference to the coupled variable's values.
    */
   _some_var_val(coupledValFace("some_var"))
{}

Real
CoupledDirichletBC::computeQpResidual()
{
  return _u_face[_qp]-(_value*_some_var_val[_qp]);
}
