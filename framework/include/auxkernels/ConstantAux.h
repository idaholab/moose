#ifndef CONSTANTAUX_H
#define CONSTANTAUX_H

#include "AuxKernel.h"


//Forward Declarations
class ConstantAux;

template<>
InputParameters valid_params<ConstantAux>();

/** 
 * Constant auxiliary value
 */
class ConstantAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  ConstantAux(std::string name,
            InputParameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to,
            std::vector<std::string> coupled_as);

  virtual ~ConstantAux() {}
  
protected:
  virtual Real computeValue();

  Real _value;
};

#endif //CONSTANTAUX_H
