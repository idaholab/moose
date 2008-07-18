#include "Kernel.h"

#ifndef BODYFORCE_H
#define BODYFORCE_H

//Forward Declarations
class BodyForce;

template<>
Parameters valid_params<BodyForce>();

class BodyForce : public Kernel
{
public:

  BodyForce(Parameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to=std::vector<std::string>(0),
                  std::vector<std::string> coupled_as=std::vector<std::string>(0))
    :Kernel(parameters,var_name,true,coupled_to,coupled_as),
     _value(_parameters.get<Real>("value"))
  {}
  
protected:
  virtual Real computeQpResidual()
  {
    return _phi[_i][_qp]*-_value;
  }

private:
  Real _value;
};
 
#endif
