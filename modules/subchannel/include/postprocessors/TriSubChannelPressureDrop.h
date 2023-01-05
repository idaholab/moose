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

#include "GeneralPostprocessor.h"
#include "TriSubChannelMesh.h"

/**
 * Calculates an overall average pressure drop for the hexagonal subchannel assembly
 */
class TriSubChannelPressureDrop : public GeneralPostprocessor
{
public:
  TriSubChannelPressureDrop(const InputParameters & params);
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual Real getValue() override;

protected:
  TriSubChannelMesh & _mesh;
  Real _value;

public:
  static InputParameters validParams();
};
