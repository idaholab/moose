#include "VelocityAux.h"

VelocityAux::VelocityAux(std::string name,
                         Parameters parameters,
                         std::string var_name,
                         std::vector<std::string> coupled_to,
                         std::vector<std::string> coupled_as)
  :AuxKernel(name, parameters, var_name, coupled_to, coupled_as),
   _p(coupledValAux("p")),
   _momentum(coupledValAux("momentum"))
{}

Real
VelocityAux::computeValue()
{
  return _momentum / _p;
}
