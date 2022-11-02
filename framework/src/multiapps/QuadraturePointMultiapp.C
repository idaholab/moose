//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "QuadraturePointMultiapp.h"
#include "MooseMesh.h"
#include "FEProblem.h"

// libMesh includes
#include "libmesh/parallel_algebra.h"
#include "libmesh/equation_systems.h"
#include <libmesh/libmesh.h>
#include <libmesh/mesh.h>
#include "libmesh/linear_implicit_system.h"
#include "libmesh/elem.h"
#include "libmesh/fe.h"

registerMooseObject("MooseApp", QuadraturePointMultiapp);

defineLegacyParams(QuadraturePointMultiapp);

InputParameters
QuadraturePointMultiapp::validParams()
{
  InputParameters params = TransientMultiApp::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription(
      "Automatically generates Sub-App positions from the elemental quadrature points of the master mesh.");
  params.suppressParameter<std::vector<Point>>("positions");
  params.suppressParameter<std::vector<FileName>>("positions_file");
  return params;
}

QuadraturePointMultiapp::QuadraturePointMultiapp(const InputParameters & parameters)
  : TransientMultiApp(parameters), BlockRestrictable(this)
{
}

void
QuadraturePointMultiapp::fillPositions()
{
/* 
 * This member function populates the array variable '_positions' with the quadrature point 
   locations of all the elements in the mesh 
*/

  MooseMesh & master_mesh = _fe_problem.mesh();
  auto & mesh = master_mesh.getMesh();
  for (auto & elem : mesh.active_local_element_ptr_range()){
    const FEFamily mapping_family = FEMap::map_fe_type(*elem);
    
    // Get a constant reference to the Finite Element type
    FEType fe_type(elem->default_order(),mapping_family);

    // Build a Finite Element object of the specified type.  Since the
    // FEBase::build() member dynamically creates memory we will
    // store the object as a std::unique_ptr<FEBase>.  This can be thought
    // of as a pointer that will clean up after itself.
    std::unique_ptr<FEBase> fe(FEBase::build(elem->dim(), fe_type));
    const int extraorder = 0;
    std::unique_ptr<QBase> qrule (fe_type.default_quadrature_rule (elem->dim(), extraorder));
  
    // Tell the finite element object to use our quadrature rule.
    fe->attach_quadrature_rule (qrule.get());

    // The physical XY locations of the quadrature points on the element.
    // These might be useful for evaluating spatially varying material
    // properties at the quadrature points.
    const std::vector<Point> & q_points = fe->get_xyz();
    fe->reinit(elem);
      
    if (hasBlocks(elem->subdomain_id())){
       for (const auto & q: q_points){
         _positions.push_back(q);
       }
    }
  }


  // Use the comm from the problem this MultiApp is part of
  libMesh::ParallelObject::comm().allgather(_positions);

  if (_positions.empty())
    mooseError("No positions found for QuadraturePointMultiapp ", _name);

  // An attempt to try to make this parallel stable
  std::sort(_positions.begin(), _positions.end());
}
