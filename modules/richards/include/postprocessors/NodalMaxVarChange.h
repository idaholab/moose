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

/**
 * Maximum change of a variable
 */
class NodalMaxVarChange : public NodalVariablePostprocessor
{
 public:
  NodalMaxVarChange(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & y);

 protected:

  /// old variable value at quad points
  VariableValue & _u_old;

  /// max(abs(_u - _u_old))
  Real _value;
};

#endif // NODALMAXVARCHANGE_H
