#ifndef REACTION_H
#define REACTION_H

#include "Kernel.h"

// Forward Declaration
class Reaction;

template<>
InputParameters validParams<Reaction>();

class Reaction : public Kernel
{
public:

  Reaction(std::string name, MooseSystem & moose_system, InputParameters parameters);
           
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

};
#endif //REACTION_H
