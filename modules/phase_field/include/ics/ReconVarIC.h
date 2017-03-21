/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RECONVARIC_H
#define RECONVARIC_H

#include "InitialCondition.h"
#include "PolycrystalICTools.h"

// Forward Declarations
class ReconVarIC;
class EBSDReader;

template <>
InputParameters validParams<ReconVarIC>();

/**
 * ReconVarIC creates a polycrystal initial condition from an EBSD dataset
 */
class ReconVarIC : public InitialCondition
{
public:
  ReconVarIC(const InputParameters & parameters);

  virtual void initialSetup();
  virtual Real value(const Point & /*p*/);

private:
  Point getCenterPoint(unsigned int grain);

  MooseMesh & _mesh;

  const EBSDReader & _ebsd_reader;

  bool _consider_phase;

  const unsigned int _phase;
  const unsigned int _op_num;
  const unsigned int _op_index;

  unsigned int _grain_num;

  const bool _all_op_elemental;
  const bool _advanced_op_assignment;

  std::vector<Point> _centerpoints;
  std::vector<unsigned int> _assigned_op;

  const std::map<dof_id_type, std::vector<Real>> & _node_to_grain_weight_map;
};

#endif // RECONVARIC_H
