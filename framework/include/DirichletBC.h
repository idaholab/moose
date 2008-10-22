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
  DirichletBC(std::string name, Parameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
    :BoundaryCondition(name, parameters, var_name, false, boundary_id, coupled_to, coupled_as),
    _value(_parameters.get<Real>("value"))
  {}
    
  virtual ~DirichletBC(){}

protected:
  virtual Real computeQpResidual()
  {
    return _u_face[_qp]-_value;
  }

private:
  /**
   * Value of u on the boundary.
   */
  Real _value;
};

#endif //DIRICHLETBC_H
