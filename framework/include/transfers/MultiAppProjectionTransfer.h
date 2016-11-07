/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#ifndef MULTIAPPPROJECTIONTRANSFER_H
#define MULTIAPPPROJECTIONTRANSFER_H

#include "MultiAppTransfer.h"
#include "libmesh/linear_implicit_system.h"

class MultiAppProjectionTransfer;

template <>
InputParameters validParams<MultiAppProjectionTransfer>();

/**
 * Project values from one domain to another
 */
class MultiAppProjectionTransfer : public MultiAppTransfer
{
public:
  MultiAppProjectionTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void execute() override;

protected:
  void toMultiApp();
  void fromMultiApp();

  void assembleL2(EquationSystems & es, const std::string & system_name);

  void projectSolution(unsigned int to_problem);

  AuxVariableName _to_var_name;
  VariableName _from_var_name;

  MooseEnum _proj_type;

  /// True, if we need to recompute the projection matrix
  bool _compute_matrix;
  std::vector<LinearImplicitSystem *> _proj_sys;
  /// Having one projection variable number seems weird, but there is always one variable in every system being used for projection,
  /// thus is always going to be 0 unless something changes in libMesh or we change the way we project variables
  unsigned int _proj_var_num;

  friend void assemble_l2(EquationSystems & es, const std::string & system_name);

  // These variables allow us to cache qps for fixed meshes.
  bool _fixed_meshes;
  bool _qps_cached;
  std::vector<std::vector<Point>> _cached_qps;
  std::vector<std::map<std::pair<unsigned int, unsigned int>, unsigned int>> _cached_index_map;
};

#endif /* MULTIAPPPROJECTIONTRANSFER_H */
