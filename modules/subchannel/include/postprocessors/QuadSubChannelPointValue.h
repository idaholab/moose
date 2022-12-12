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
#include "QuadSubChannelMesh.h"

/**
 * Calculates a user selected variable at a user selected point in the assembly
 */
class QuadSubChannelPointValue : public GeneralPostprocessor
{
public:
  QuadSubChannelPointValue(const InputParameters & params);
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual Real getValue() override;

protected:
  QuadSubChannelMesh & _mesh;
  const Real & _height;
  const int & _ix;
  const int & _iy;
  Point _point;
  const unsigned int _var_number;
  const System & _system;
  Real _value;

public:
  static InputParameters validParams();
};
