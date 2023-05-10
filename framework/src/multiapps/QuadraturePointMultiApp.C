//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/main/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "QuadraturePointMultiApp.h"
#include "MooseMesh.h"
#include "FEProblem.h"

// libMesh includes
#include "libmesh/parallel_algebra.h"
#include "libmesh/mesh.h"
#include "libmesh/elem.h"
#include "libmesh/fe.h"

registerMooseObject("MooseApp", QuadraturePointMultiApp);
// TODO: Deprecate and use Positions system

InputParameters
QuadraturePointMultiApp::validParams()
{
  InputParameters params = TransientMultiApp::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription(
      "Automatically generates sub-App positions from the elemental quadrature points, with the "
      "default quadrature, in the parent mesh.");
  params.suppressParameter<std::vector<Point>>("positions");
  params.suppressParameter<std::vector<FileName>>("positions_file");
  params.suppressParameter<std::vector<PositionsName>>("positions_objects");
  return params;
}

QuadraturePointMultiApp::QuadraturePointMultiApp(const InputParameters & parameters)
  : TransientMultiApp(parameters), BlockRestrictable(this)
{
}

void
QuadraturePointMultiApp::fillPositions()
{
  MooseMesh & main_mesh = _fe_problem.mesh();
  auto & mesh = main_mesh.getMesh();
  for (auto & elem : mesh.active_local_element_ptr_range())
  {
    // FEType is local to the element, supporting mixed order meshes
    const FEFamily mapping_family = FEMap::map_fe_type(*elem);
    FEType fe_type(elem->default_order(), mapping_family);
    const auto qrule = fe_type.default_quadrature_rule(elem->dim());

    // Build a Finite Element object of the specified type
    std::unique_ptr<FEBase> fe(FEBase::build(elem->dim(), fe_type));

    // Tell the finite element object to use our quadrature rule.
    fe->attach_quadrature_rule(qrule.get());

    // The physical XY locations of the quadrature points on the element.
    // These might be useful for evaluating spatially varying material
    // properties at the quadrature points.
    const std::vector<Point> & q_points = fe->get_xyz();
    fe->reinit(elem);

    if (hasBlocks(elem->subdomain_id()))
      for (const auto & q : q_points)
        _positions.push_back(q);
  }

  // Remove local duplicates from the vector of positions as transfers will not handle them
  std::sort(_positions.begin(), _positions.end());
  _positions.erase(std::unique(_positions.begin(), _positions.end()), _positions.end());

  // Use the comm from the problem this MultiApp is part of
  libMesh::ParallelObject::comm().allgather(_positions);

  // Remove duplicates occurring at process boundaries
  std::sort(_positions.begin(), _positions.end());
  _positions.erase(std::unique(_positions.begin(), _positions.end()), _positions.end());

  if (_positions.empty())
    mooseError("No positions found for QuadraturePointMultiApp ", _name);
}
