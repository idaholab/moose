#include "BoundaryCondition.h"

#ifndef COUPLEDDIRICHLETBC_H
#define COUPLEDDIRICHLETBC_H

//Forward Declarations
class CoupledDirichletBC;

template<>
InputParameters validParams<CoupledDirichletBC>();

/**
 * Implements a coupled Dirichlet BC where u = value * some_var on the boundary.
 */
class CoupledDirichletBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  CoupledDirichletBC(std::string name, MooseSystem & moose_system, InputParameters parameters);
    
  virtual ~CoupledDirichletBC(){}

protected:
  virtual Real computeQpResidual();

private:
  /**
   * Multiplier on the boundary.
   */
  Real _value;

  /**
   * Holds the values at the quadrature points
   * of a coupled variable.
   */
  MooseArray<Real> & _some_var_val;
};

#endif //COUPLEDDIRICHLETBC_H
