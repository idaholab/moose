//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActionComponent.h"
#include "ComponentMeshTransformHelper.h"
#include "GravityInterface.h"

class FlowChannel1PhasePhysics;

/**
 * A 1D single-phase flow channel.
 *
 * The mesh is generated through the MeshGenerator system (see addMeshGenerators()). The governing
 * equations are defined by an internally-created FlowChannel1PhasePhysics, built and registered by
 * this component's constructor (rather than requiring the user to declare a separate [Physics]
 * block), so that an input file only needs to rename [Components] to [ActionComponents] and
 * FlowChannel1Phase to FlowChannel1PhaseAC to move a plain, unheated, frictionless flow channel
 * over to the native ActionComponent - see FlowChannel1PhasePhysics for what is and is not
 * currently covered.
 */
class FlowChannel1PhaseAC : public virtual ActionComponent,
                            public ComponentMeshTransformHelper,
                            public GravityInterface
{
public:
  static InputParameters validParams();
  FlowChannel1PhaseAC(const InputParameters & params);

  /// The physics this component created for itself, e.g. for a boundary condition component to
  /// look up this flow channel's fluid properties/numerical flux user object names
  const FlowChannel1PhasePhysics & getPhysics() const;

protected:
  virtual void addMeshGenerators() override;
  virtual void addPhysics() override;

  /// Length of the flow channel
  const Real _length;
  /// Number of elements along the flow channel
  const unsigned int _n_elems;

  /// The physics this component creates and owns; attaches itself to this component in addPhysics()
  FlowChannel1PhasePhysics * _physics;
};
