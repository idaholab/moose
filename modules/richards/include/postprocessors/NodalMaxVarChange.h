/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef NODALMAXVARCHANGE_H
#define NODALMAXVARCHANGE_H

#include "NodalVariablePostprocessor.h"

class MooseVariable;

//Forward Declarations
class NodalMaxVarChange;

template<>
InputParameters validParams<NodalMaxVarChange>();

class NodalMaxVarChange : public NodalVariablePostprocessor
{
 public:
  NodalMaxVarChange(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & y);

 protected:

  VariableValue & _u_old;   // Holds the old variable at current quadrature points
  Real _value; // holds max(abs(_u - _u_old))
};

#endif // NODALMAXVARCHANGE_H
