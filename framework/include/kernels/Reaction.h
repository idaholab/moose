#ifndef REACTION_H
#define REACTION_H

#include "Kernel.h"


//Forward Declarations
class Reaction;

class Reaction : public Kernel
{
public:

  Reaction(std::string name,
           InputParameters parameters,
           std::string var_name,
           std::vector<std::string> coupled_to=std::vector<std::string>(0),
           std::vector<std::string> coupled_as=std::vector<std::string>(0));

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

};
#endif //REACTION_H
