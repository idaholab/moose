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

/**
 * Action for building empty mesh object for hexagonal geometry that is filled by mesh
 * generators
 */
class TriSubChannelBuildMeshAction : public Action
{
public:
  TriSubChannelBuildMeshAction(const InputParameters & params);

  virtual void act();

public:
  static InputParameters validParams();
};
