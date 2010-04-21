#ifndef COUPLEDAUX_H
#define COUPLEDAUX_H

#include "AuxKernel.h"


//Forward Declarations
class CoupledAux;

template<>
InputParameters validParams<CoupledAux>();

/** 
 * Coupled auxiliary value
 */
class CoupledAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  CoupledAux(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~CoupledAux() {}
  
protected:
  virtual Real computeValue();

  Real _value;
  std::string _operator;

  int _coupled;
  Real & _coupled_val;
};

#endif //COUPLEDAUX_H
