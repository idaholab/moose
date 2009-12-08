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
  DirichletBC(std::string name, InputParameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as);
    
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
