#include "BoundaryCondition.h"

#ifndef COUPLEDDIRICHLETBC_H
#define COUPLEDDIRICHLETBC_H

//Forward Declarations
class CoupledDirichletBC;

template<>
InputParameters valid_params<CoupledDirichletBC>();

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
  CoupledDirichletBC(std::string name,
                     InputParameters parameters,
                     std::string var_name,
                     unsigned int boundary_id,
                     std::vector<std::string> coupled_to,
                     std::vector<std::string> coupled_as);
    
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
  std::vector<Real> & _some_var_val;
};

#endif //COUPLEDDIRICHLETBC_H
