//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "BlockRestrictable.h"
#include "INSFVPressureVariable.h"
#include "NonADFunctorInterface.h"

class MooseMesh;
class INSFVPressureVariable;

/**
 * This user-object corrects the pressure
 */
class NSPressurePin : public GeneralUserObject,
                      public BlockRestrictable,
                      public NonADFunctorInterface
{
public:
  static InputParameters validParams();
  NSPressurePin(const InputParameters & params);

  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// LibMesh mesh class for the current simulation mesh
  MeshBase & _mesh;

  /// The thread 0 copy of the pressure variable
  const MooseVariableFieldBase & _p;

  /// Value of the pressure pin
  const PostprocessorValue & _p0;

  /// Pressure pin type
  const MooseEnum _pressure_pin_type;

  /// If using point-value pressure pin, the point at which to apply the pin
  const Point _pressure_pin_point;

  /// If using average pressure pin, provides the average pressure value
  const PostprocessorValue * const _current_pressure_average;

private:
  /// The nonlinear system
  SystemBase & _sys;
};
