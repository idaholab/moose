//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include "libmesh/parallel.h"
#include "libmesh/fe_type.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/equation_systems.h"
#include "libmesh/explicit_system.h"
#include "libmesh/point_locator_base.h"

class ParameterMesh
{
public:
  ParameterMesh(const FEType & param_type, const std::string & exodus_mesh);

  dof_id_type size() const { return _sys->n_dofs(); }

  void getIndexAndWeight(const Point & pt,
                         std::vector<dof_id_type> & dof_indices,
                         std::vector<Real> & weights) const;

  void getIndexAndWeight(const Point & pt,
                         std::vector<dof_id_type> & dof_indices,
                         std::vector<RealGradient> & weights) const;

protected:
  Parallel::Communicator _communicator;
  ReplicatedMesh _mesh;
  std::unique_ptr<EquationSystems> _eq;
  System * _sys;
  std::unique_ptr<PointLocatorBase> _point_locator;
};
