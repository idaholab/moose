#ifndef NEUMANNBC_H
#define NEUMANNBC_H

#include "BoundaryCondition.h"


//Forward Declarations
class NeumannBC;

template<>
InputParameters validParams<NeumannBC>();

/**
 * Implements a simple constant Neumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class NeumannBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NeumannBC(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
virtual ~NeumannBC() {}

protected:
  virtual Real computeQpResidual();
  
private:
  /**
   * Value of grad(u) on the boundary.
   */
  Real _value;
};

#endif //NEUMANNBC_H
