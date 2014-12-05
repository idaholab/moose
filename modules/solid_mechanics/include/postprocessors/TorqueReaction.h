/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
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
  TorqueReaction(const std::string & name, InputParameters parameters);

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

  VariableValue & _react_x;
  VariableValue & _react_y;
  VariableValue & _react_z;

  const Point _axis_origin;
  const Point _axis_direction;

  Real _sum;
};

#endif
