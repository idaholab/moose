#include "BoundaryCondition.h"

#ifndef DIRICHLETBC_H
#define DIRICHLETBC_H

class DirichletBC : public BoundaryCondition
{
public:
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
  Real _value;
};

#endif //DIRICHLETBC_H
