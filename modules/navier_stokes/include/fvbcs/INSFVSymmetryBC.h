//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

class InputParameters;

/**
 * A parent class for INSFV symmetry boundary conditions
 */
class INSFVSymmetryBC
{
public:
  static InputParameters validParams();
  INSFVSymmetryBC(const InputParameters & params);
  virtual ~INSFVSymmetryBC() = default;
};
