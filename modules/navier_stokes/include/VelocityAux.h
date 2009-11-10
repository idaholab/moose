#ifndef VELOCITYAUX_H
#define VELOCITYAUX_H

#include "AuxKernel.h"

/** 
 * Velocity auxiliary value
 */
class VelocityAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  VelocityAux(std::string name,
            InputParameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to,
            std::vector<std::string> coupled_as);

  virtual ~VelocityAux() {}
  
protected:
  virtual Real computeValue();

  Real & _p;
  Real & _momentum;
};

#endif //VELOCITYAUX_H
