#ifndef DGBC_H
#define DGBC_H

#include "BoundaryCondition.h"


//Forward Declarations
class DGBC;
class Function;

template<>
InputParameters validParams<DGBC>();

/**
 * Implements a simple constant Dirichlet BC where u=value on the boundary.
 */
class DGBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  DGBC(std::string name, MooseSystem & moose_system, InputParameters parameters);
    
  virtual ~DGBC() {}

protected:
  virtual Real computeQpResidual();

private:
  /**
   * Value of u on the boundary.
   */
  Real _value;

  Function & _func;

  Real _pen;
};

#endif //DGBC_H
