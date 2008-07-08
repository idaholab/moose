#include "BoundaryCondition.h"

#ifndef DIRICHLETBC_H
#define DIRICHLETBC_H

//Forward Declarations
class DirichletBC;

template<>
Parameters valid_params<DirichletBC>()
{
  Parameters params;
  params.set<Real>("value")=0.0;
  return params;
}

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
  DirichletBC(Parameters parameters, EquationSystems * es, std::string var_name, unsigned int boundary_id)
    :BoundaryCondition(parameters, es, var_name, false, boundary_id),
     _value(_parameters.get<Real>("value"))
  {}

  /**
   * Standalone constructor.
   */
  DirichletBC(EquationSystems * es, std::string var_name, unsigned int boundary_id, Real value)
    :BoundaryCondition(es, var_name, false, boundary_id),
     _value(value)
  {}
    
  virtual ~DirichletBC(){}

protected:
  virtual Real computeQpResidual()
  {
    return _u-_value;
  }

private:
  /**
   * Value of u on the boundary.
   */
  Real _value;
};

#endif //DIRICHLETBC_H
