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
#include "TaggingInterface.h"
#include "BlockRestrictable.h"
#include "ADReal.h"
#include "MooseTypes.h"
#include "CellCenteredMapFunctor.h"
#include "VectorComponentFunctor.h"
#include "FaceArgInterface.h"
#include "INSFVPressureVariable.h"
#include "ADFunctorInterface.h"

#include "libmesh/vector_value.h"
#include "libmesh/id_types.h"
#include "libmesh/stored_range.h"
#include <unordered_map>
#include <set>
#include <unordered_set>

class MooseMesh;
class INSFVVelocityVariable;
class INSFVPressureVariable;
namespace libMesh
{
class Elem;
class MeshBase;
}

/**
 * This user-object corrects the pressure
 */
class NSFVPressurePin : public GeneralUserObject,
                        public TaggingInterface,
                        public BlockRestrictable,
                        public FaceArgProducerInterface,
                        public ADFunctorInterface
{
public:
  static InputParameters validParams();
  NSFVPressurePin(const InputParameters & params);

  void initialSetup() override;
  void execute() override;

protected:
  /// The thread 0 copy of the pressure variable
  INSFVPressureVariable * const _p;

  /// Pressure pin type
  const MooseEnum _pressure_pin_type;

  /// If using point-value pressure pin, the point at which to apply the pin
  const Point _pressure_pin_point;

  /// If using average pressure pin, provides the average pressure value
  PostprocessorValue * const _current_pressure_average;

private:
  /// The nonlinear system
  SystemBase & _sys;
};
