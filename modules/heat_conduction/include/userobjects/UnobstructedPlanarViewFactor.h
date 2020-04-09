//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ViewFactorBase.h"

// Forward Declarations

/**
 * Computes the view factors for planar faces in unobstructed radiative heat transfer
 */
class UnobstructedPlanarViewFactor : public ViewFactorBase
{
public:
  static InputParameters validParams();

  UnobstructedPlanarViewFactor(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override;

protected:
  virtual void threadJoinViewFactor(const UserObject & y) override;
  virtual void finalizeViewFactor() override;

  /// helper function that reinits an element face
  void reinitFace(dof_id_type elem_id, unsigned int side);

  BoundaryInfo * _boundary_info;
  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> _side_list;

  ///@{ data of the to_elem side being initialized
  std::unique_ptr<const Elem> _current_remote_side;
  std::unique_ptr<FEBase> _current_remote_fe;
  Real _current_remote_side_volume;
  const std::vector<Real> * _current_remote_JxW;
  const std::vector<Point> * _current_remote_xyz;
  const std::vector<Point> * _current_remote_normals;
  std::vector<Real> _current_remote_coord;
  ///@}

  unsigned int _exponent;
  Real _divisor;
};
