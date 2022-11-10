/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#pragma once

#include "MooseObjectAction.h"

/**
 * Action that creates SubChannel problem
 */
class SubChannelCreateProblemAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  SubChannelCreateProblemAction(const InputParameters & parameters);

  virtual void act() override;
};
