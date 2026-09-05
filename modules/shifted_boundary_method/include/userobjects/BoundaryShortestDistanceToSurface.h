//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideUserObject.h"

#include <vector>

class Function;
class SBMInterfaceManager;

class BoundaryShortestDistanceToSurface : public SideUserObject
{
public:
  using ElemSide = std::pair<dof_id_type, unsigned short int>;

  static InputParameters validParams();
  BoundaryShortestDistanceToSurface(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void initialize() override;
  virtual void threadJoin(const UserObject & uo) override;

  /// Provides query interfaces for distance vector
  const RealVectorValue & surrogateDistance(const ElemSide & elem_side, unsigned int qp) const;
  /// Provides query interfaces for normal vector
  const RealVectorValue & trueNormal(const ElemSide & elem_side, unsigned int qp) const;

protected:
  /// Local distance functions when no external distance user object is supplied.
  std::vector<const Function *> _distance_functions;

  /// distance
  std::map<ElemSide, std::vector<RealVectorValue>> _distance_vectors;

  /// normal
  mutable std::map<ElemSide, std::vector<RealVectorValue>> _normal_vectors;

  /// side id to index map
  std::map<BoundaryID, unsigned int> _side_id_index;

  /// Optional manager supplying geometry for interfaces in a single surface mesh.
  const SBMInterfaceManager * _manager = nullptr;

  /// Ordered subdomain pair associated with each surrogate boundary.
  std::map<BoundaryID, std::pair<SubdomainID, SubdomainID>> _boundary_subdomain_pairs;

private:
  /// Signed true-interface measure integrated over each surrogate boundary.
  std::vector<Real> _true_interface_measures;

  /// Map from ElemSide to boundary ID index.
  std::map<ElemSide, unsigned int> _elem_side_to_bid;

  /// @brief Whether to suppress warnings about large distances.
  bool _suppress_distance_warning;

  /// @brief Whether to output debug information.
  bool _debug_output;
};
