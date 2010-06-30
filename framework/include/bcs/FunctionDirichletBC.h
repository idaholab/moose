#ifndef FUNCTIONDIRICHLETBC_H
#define FUNCTIONDIRICHLETBC_H

#include "BoundaryCondition.h"
#include "Functor.h"

//Forward Declarations
class FunctionDirichletBC;

template<>
InputParameters validParams<FunctionDirichletBC>();

/**
 * Defines a boundary condition that forces the value to be a user specified
 * function at the boundary.
 */
class FunctionDirichletBC : public BoundaryCondition
{
public:

  FunctionDirichletBC(std::string name,
             MooseSystem &sys,
             InputParameters parameters);

protected:
  /**
   * Evaluate f at the current quadrature point.
   */
  Real f();

  /**
   * u - the function
   */
  virtual Real computeQpResidual();

private:
  Functor _functor;
};
#endif //FUNCTIONDIRICHLETBC_H
