//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

/**
 *  Base class for subcritical crack growth reporters
 */
class CrackMeshCut3DUserObject;
class CrackGrowthReporterBase : public GeneralReporter
{
public:
  static InputParameters validParams();
  CrackGrowthReporterBase(const InputParameters & parameters);
  virtual void execute() override final;
  virtual void initialize() override final {}
  virtual void finalize() override final {}

protected:
  /**
   * derived classes compute specific crack growth increment
   * @param index index vector from the cutter mesh.
   */
  virtual void compute_growth(std::vector<int> & index) = 0;

  /// cutter mesh object name
  const UserObjectName & _cutter_name;
  /// 3D mesh cutter object that provides active nodes
  CrackMeshCut3DUserObject * _3Dcutter;
  /// Length of crack growth for each fracture integral value
  const Real _max_growth_size;

  /// The name of the reporter with fracture integral values
  const std::vector<Real> & _ki_vpp;

  ///@{ The variables with the x, y, z, id data where ki was computed
  const std::vector<Real> & _ki_x;
  const std::vector<Real> & _ki_y;
  const std::vector<Real> & _ki_z;
  const std::vector<Real> & _ki_id;
  ///@}

  ///@{ The variables with the x, y, z data for output
  std::vector<Real> & _x;
  std::vector<Real> & _y;
  std::vector<Real> & _z;
  std::vector<Real> & _id;
  ///@}

private:
  /**
   * get indexing from the cutter mesh
   * @returns the cutter mesh index vector
   */
  std::vector<int> get_cutter_mesh_indices() const;
  /// copy data into coordinate reporters
  void copy_coordinates() const;
};
