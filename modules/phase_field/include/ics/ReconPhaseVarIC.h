/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RECONPHASEVARIC_H
#define RECONPHASEVARIC_H

#include "InitialCondition.h"
#include "EBSDReader.h"
#include "PolycrystalICTools.h"

// Forward Declarations
class ReconPhaseVarIC;

template <>
InputParameters validParams<ReconPhaseVarIC>();

/**
 * ReconPhaseVarIC initializes a single order parameter to represent a phase
 * obtained form an EBSDReader object. The op will be set to 1 for nodes that are
 * entirely surrounded by the given phase and 0 for nodes that touch no elements
 * with the selected phase (and fractional values for nodes on interfaces)
 */
class ReconPhaseVarIC : public InitialCondition
{
public:
  ReconPhaseVarIC(const InputParameters & parameters);

  virtual Real value(const Point & /*p*/);

private:
  MooseMesh & _mesh;

  const EBSDReader & _ebsd_reader;

  unsigned int _phase;

  const std::map<dof_id_type, std::vector<Real>> & _node_to_phase_weight_map;
};

#endif // RECONPHASEVARIC_H
