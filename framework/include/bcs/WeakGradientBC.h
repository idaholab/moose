#ifndef WEAKGRADIENTBC_H
#define WEAKGRADIENTBC_H

#include "BoundaryCondition.h"


//Forward Declarations
class WeakGradientBC;

template<>
InputParameters validParams<WeakGradientBC>();

/**
 * Implements a simple constant Neumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class WeakGradientBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  WeakGradientBC(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
virtual ~WeakGradientBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  
private:
  /**
   * Value of grad(u) on the boundary.
   */
  Real _value;
};

#endif //WEAKGRADIENTBC_H
