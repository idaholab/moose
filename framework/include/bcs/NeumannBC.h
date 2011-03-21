#ifndef NEUMANNBC_H
#define NEUMANNBC_H

#include "IntegratedBC.h"


/**
 * Implements a simple constant Neumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class NeumannBC : public IntegratedBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NeumannBC(const std::string & name, InputParameters parameters);
  

protected:
  virtual Real computeQpResidual();
  
  /**
   * Value of grad(u) on the boundary.
   */
  Real _value;
};


template<>
InputParameters validParams<NeumannBC>();

#endif //NEUMANNBC_H_
