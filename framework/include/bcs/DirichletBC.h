#ifndef DIRICHLETBC_H
#define DIRICHLETBC_H

#include "BoundaryCondition.h"


//Forward Declarations
class DirichletBC;

template<>
InputParameters validParams<DirichletBC>();

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
  DirichletBC(std::string name, MooseSystem & moose_system, InputParameters parameters);
    
  virtual ~DirichletBC() {}

protected:
  virtual Real computeQpResidual();

private:
  /**
   * Value of u on the boundary.
   */
  Real _value;
};

#endif //DIRICHLETBC_H
