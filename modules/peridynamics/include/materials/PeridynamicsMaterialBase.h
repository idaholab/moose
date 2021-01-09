//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "PeridynamicsMesh.h"

/**
 * Base class for peridynamics material models
 */
class PeridynamicsMaterialBase : public Material
{
public:
  static InputParameters validParams();

  PeridynamicsMaterialBase(const InputParameters & parameters);

protected:
  /**
   * Function to setup mesh related data to be used in this class
   */
  void setupMeshRelatedData();

  ///@{ Mesh related information for material points of current bond/element
  PeridynamicsMesh & _pdmesh;
  const unsigned int _dim;
  const unsigned int _nnodes;
  std::vector<Real> _horizon_radius;
  std::vector<Real> _node_vol;
  std::vector<Real> _horizon_vol;

  RealGradient _origin_vec;
  ///@}
};
