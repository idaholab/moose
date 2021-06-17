//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"
#include "EBSDReader.h"
#include "PolycrystalICTools.h"

/**
 * ReconPhaseVarIC initializes a single order parameter to represent a phase
 * obtained form an EBSDReader object. The op will be set to 1 for nodes that are
 * entirely surrounded by the given phase and 0 for nodes that touch no elements
 * with the selected phase (and fractional values for nodes on interfaces)
 */
class ReconPhaseVarIC : public InitialCondition
{
public:
  static InputParameters validParams();

  ReconPhaseVarIC(const InputParameters & parameters);

  virtual Real value(const Point & /*p*/);

private:
  MooseMesh & _mesh;
  const EBSDReader & _ebsd_reader;
  unsigned int _phase;
  const std::map<dof_id_type, std::vector<Real>> & _node_to_phase_weight_map;
};
