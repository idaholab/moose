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

#include "Action.h"
#include "MooseEnum.h"

/**
 * Action that adds SubChannel variables needs for the solve
 */
class SubChannelAddVariablesAction : public Action
{
public:
  static InputParameters validParams();

  SubChannelAddVariablesAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  /// FE family of the aux variables added by this action
  MooseEnum _fe_family;
  /// FE order of the aux variables added by this action
  MooseEnum _fe_order;
};
