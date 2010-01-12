#ifndef BODYFORCE_H
#define BODYFORCE_H

#include "Kernel.h"

//Forward Declarations
class BodyForce;

template<>
InputParameters validParams<BodyForce>();

class BodyForce : public Kernel
{
public:

  BodyForce(std::string name,
            InputParameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to=std::vector<std::string>(0),
            std::vector<std::string> coupled_as=std::vector<std::string>(0));
  
protected:
  virtual Real computeQpResidual();

private:
  Real _value;
};
 
#endif
