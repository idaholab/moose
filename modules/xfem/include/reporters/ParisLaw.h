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
 *  ParisLaw is a reporter that compute fracture growth size and number of cycles
 */
class CrackMeshCut3DUserObject;
class ParisLaw : public GeneralReporter
{
public:
  static InputParameters validParams();
  ParisLaw(const InputParameters & parameters);
  virtual void execute() override;
  virtual void initialize() override {}
  virtual void finalize() override {}

protected:
  /// cutter mesh object name
  const UserObjectName & _cutter_name;
  /// 3D mesh cutter object that provides active nodes
  CrackMeshCut3DUserObject * _3Dcutter;
  /// Length of crack growth for each fracture integral value
  const Real _max_growth_size;
  /// Paris law parameters
  const Real _paris_law_c;
  const Real _paris_law_m;

  /// The name of the reporter with fracture integral values
  const std::vector<Real> & _ki_vpp;
  const std::vector<Real> & _kii_vpp;
  ///@{ The variables with the x, y, z, id data where ki was computed
  const std::vector<Real> & _ki_x;
  const std::vector<Real> & _ki_y;
  const std::vector<Real> & _ki_z;
  const std::vector<Real> & _ki_id;
  ///@}

  /// Number of cycles vector to reach max_growth_size postprocessor
  Real & _dn;

  /// growth rate reporter
  std::vector<Real> & _growth_increment;

  ///@{ The variables with the x, y, z data for output
  std::vector<Real> & _x;
  std::vector<Real> & _y;
  std::vector<Real> & _z;
  std::vector<Real> & _id;
  ///@}
};
