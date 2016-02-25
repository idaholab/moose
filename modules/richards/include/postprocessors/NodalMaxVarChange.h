/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


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
  NodalMaxVarChange(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & y);

protected:

  /// old variable value at quad points
  const VariableValue & _u_old;

  /// max(abs(_u - _u_old))
  Real _value;
};

#endif // NODALMAXVARCHANGE_H
