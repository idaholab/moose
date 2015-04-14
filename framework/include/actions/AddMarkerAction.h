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

#ifndef ADDMARKERACTION_H
#define ADDMARKERACTION_H

#include "MooseObjectAction.h"

class AddMarkerAction;

template<>
InputParameters validParams<AddMarkerAction>();


class AddMarkerAction : public MooseObjectAction
{
public:
  AddMarkerAction(InputParameters params);
  AddMarkerAction(const std::string & deprecated_name, InputParameters params); // DEPRECATED CONSTRUCTOR

  virtual void act();

private:

};

#endif // ADDMARKERACTION_H
