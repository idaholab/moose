#include "BoundaryCondition.h"

#ifndef COUPLEDDIRICHLETBC_H
#define COUPLEDDIRICHLETBC_H

//Forward Declarations
class CoupledDirichletBC;

template<>
Parameters valid_params<CoupledDirichletBC>();

/**
 * Implements a simple constant CoupledDirichlet BC where u=value on the boundary.
 */
class CoupledDirichletBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  CoupledDirichletBC(std::string name, Parameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
    :BoundaryCondition(name, parameters, var_name, false, boundary_id, coupled_to, coupled_as),
    _value(_parameters.get<Real>("value")),
    _some_var_val(coupledValFace("some_var"))
  {}
    
  virtual ~CoupledDirichletBC(){}

protected:
  virtual Real computeQpResidual()
  {
    return _u_face[_qp]-(_value*_some_var_val[_qp]);
  }

private:
  /**
   * Multiplier on the boundary.
   */
  Real _value;

  /**
   * Holds the values at the quadrature points
   * of a coupled variable.
   */
  std::vector<Real> & _some_var_val;
};

#endif //COUPLEDDIRICHLETBC_H
