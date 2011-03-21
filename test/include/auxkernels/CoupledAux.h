#ifndef COUPLEDAUX_H_
#define COUPLEDAUX_H_

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
  CoupledAux(const std::string & name, InputParameters parameters);

  virtual ~CoupledAux() {}
  
protected:
  virtual Real computeValue();

  Real _value;
  std::string _operator;

  int _coupled;
  VariableValue & _coupled_val;
};

#endif //COUPLEDAUX_H_
