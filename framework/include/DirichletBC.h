#include "BoundaryCondition.h"

#ifndef DIRICHLETBC_H
#define DIRICHLETBC_H

//Forward Declarations
class DirichletBC;

template<>
Parameters valid_params<DirichletBC>();

/**
 * Implements a simple constant Dirichlet BC where u=value on the boundary.
 */
class DirichletBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  DirichletBC(Parameters parameters, std::string var_name, unsigned int boundary_id)
    :BoundaryCondition(parameters, var_name, false, boundary_id),
     _value(_parameters.get<Real>("value"))
  {}
    
  virtual ~DirichletBC(){}

protected:
  virtual Real computeQpResidual()
  {
    return _u[_qp]-_value;
  }

  virtual Real computeQpJacobian()
  {
    return 1;
  }

private:
  /**
   * Value of u on the boundary.
   */
  Real _value;
};

#endif //DIRICHLETBC_H
