/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMMONTENSORMECHANICSACTION_H
#define COMMONTENSORMECHANICSACTION_H

#include "Action.h"

class CommonTensorMechanicsAction;

template <>
InputParameters validParams<CommonTensorMechanicsAction>();

/**
 * Store common tensor mechanics parameters
 */
class CommonTensorMechanicsAction : public Action
{
public:
  CommonTensorMechanicsAction(const InputParameters & parameters);

  virtual void act() override{};
};

#endif // COMMONTENSORMECHANICSACTION_H
