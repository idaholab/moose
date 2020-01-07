//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideUserObject.h"

// Forward Declarations
class ViewFactorBase;

template <>
InputParameters validParams<ViewFactorBase>();

/**
 * A base class for automatic computation of view factors between sidesets
 */
class ViewFactorBase : public SideUserObject
{
public:
  static InputParameters validParams();

  ViewFactorBase(const InputParameters & parameters);

  ///@{ public interface for obtaining view factors
  Real getViewFactor(BoundaryID from_id, BoundaryID to_id) const;
  Real getViewFactor(BoundaryName from_name, BoundaryName to_name) const;
  ///@}

  virtual void finalize() override final;

protected:
  virtual void threadJoin(const UserObject & y) override final;

  /// this function checks & normalizes view factors to sum to one, this is not always
  void checkAndNormalizeViewFactor();

  /// a purely virtural function called in finalize, must be overriden by derived class
  virtual void finalizeViewFactor() = 0;

  /// a purely virtural function called in finalize, must be overriden by derived class
  virtual void threadJoinViewFactor(const UserObject & y) = 0;

  /// number of boundaries of this side uo
  unsigned int _n_sides;

  /// area of the sides i
  std::vector<Real> _areas;

  /// view factor tolerance
  const Real _view_factor_tol;

  /// whether to normalize view factors so vf[from][:] sums to one
  const bool _normalize_view_factor;

  /// the view factor from side i to side j
  std::vector<std::vector<Real>> _view_factors;

  /// boundary name to index map
  std::unordered_map<std::string, unsigned int> _side_name_index;
};
