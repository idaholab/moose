#ifndef COUPLEDAUX_H
#define COUPLEDAUX_H

#include "AuxKernel.h"


//Forward Declarations
class CoupledAux;

template<>
InputParameters valid_params<CoupledAux>();

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
  CoupledAux(std::string name,
            InputParameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to,
            std::vector<std::string> coupled_as);

  virtual ~CoupledAux() {}
  
protected:
  virtual Real computeValue();

  Real _value;
  std::string _operator;

  int _coupled;
  Real & _coupled_val;
};

#endif //COUPLEDAUX_H
