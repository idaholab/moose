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
   * Compute crack growth increment at the specified crack front point and store increments
   * in an internal data structure.
   * @param index Vector of crack front point indices from the cutter mesh.
   */
  virtual void computeGrowth(std::vector<int> & index) = 0;

  /// cutter mesh object name
  const UserObjectName & _cutter_name;
  /// 3D mesh cutter object that provides active nodes
  CrackMeshCut3DUserObject * _3Dcutter;
  /// Maximum crack growth increment allowed for any of the crack front points
  const Real _max_growth_increment;

  /// The name of the reporter with K_I fracture integral values
  const std::vector<Real> & _ki_vpp;

  ///@{
  /// Crack front point locations where fracture integrals are computed, stored as the
  /// x, y, and z coordinates and position along the crack front (id)
  const std::vector<Real> & _ki_x;
  const std::vector<Real> & _ki_y;
  const std::vector<Real> & _ki_z;
  const std::vector<Real> & _ki_id;
  ///@}

  ///@{
  /// Crack front point locations for output, stored as x, y, and z coordinates and position
  /// along the crack front (id)
  std::vector<Real> & _x;
  std::vector<Real> & _y;
  std::vector<Real> & _z;
  std::vector<Real> & _id;
  ///@}

private:
  /**
   * get indexing from the cutter mesh
   * @return the cutter mesh index vector
   */
  std::vector<int> getCutterMeshIndices() const;
  /// copy data into coordinate reporters
  void copyCoordinates() const;
};
