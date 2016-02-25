/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef TORQUEREACTION_H
#define TORQUEREACTION_H

#include "NodalPostprocessor.h"

//Forward Declarations
class TorqueReaction;
class AuxiliarySystem;

template<>
InputParameters validParams<TorqueReaction>();

class TorqueReaction :
  public NodalPostprocessor
{
public:
  TorqueReaction(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  void threadJoin(const UserObject & y);

protected:

  AuxiliarySystem & _aux;
//  std::vector<AuxVariableName> _reaction_vars;
  MooseVariable & _react_x_var;
  MooseVariable & _react_y_var;
  MooseVariable & _react_z_var;

  const VariableValue & _react_x;
  const VariableValue & _react_y;
  const VariableValue & _react_z;

  const Point _axis_origin;
  const Point _axis_direction;

  Real _sum;
};

#endif
