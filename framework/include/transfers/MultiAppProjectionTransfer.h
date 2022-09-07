//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppConservativeTransfer.h"

// Forward declarations
namespace libMesh
{
class LinearImplicitSystem;
}

/**
 * Project values from one domain to another
 */
class MultiAppProjectionTransfer : public MultiAppConservativeTransfer
{
public:
  static InputParameters validParams();

  MultiAppProjectionTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void execute() override;

protected:
  void toMultiApp();
  void fromMultiApp();

  void assembleL2(EquationSystems & es, const std::string & system_name);

  void projectSolution(unsigned int to_problem);

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
  std::map<processor_id_type, std::vector<Point>> _cached_qps;
  std::map<processor_id_type, std::map<std::pair<unsigned int, unsigned int>, unsigned int>>
      _cached_index_map;

private:
  bool usesMooseAppCoordTransform() const override { return true; }
};
