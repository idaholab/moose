#ifndef SINNEUMANNBC_H
#define SINNEUMANNBC_H

#include "BoundaryCondition.h"


//Forward Declarations
class SinNeumannBC;

template<>
InputParameters validParams<SinNeumannBC>();

/**
 * Implements a simple constant SinNeumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class SinNeumannBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  SinNeumannBC(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
virtual ~SinNeumannBC() {}

protected:
  virtual Real computeQpResidual();
  
private:
  Real _initial;
  Real _final;
  Real _duration;

  /**
   * Value of grad(u) on the boundary.
   */
  Real _value;
};

#endif //SINNEUMANNBC_H
