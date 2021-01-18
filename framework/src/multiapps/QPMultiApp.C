//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "QPMultiApp.h"
#include "MooseMesh.h"
#include "FEProblem.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "libmesh/quadrature.h"
// libMesh includes
#include "libmesh/parallel_algebra.h"
#include "MultiApp.h"
#include "MultiAppTransfer.h"

registerMooseObject("MooseApp", QPMultiApp);

defineLegacyParams(QPMultiApp);

InputParameters
QPMultiApp::validParams()
{
  InputParameters params = TransientMultiApp::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription(
      "Automatically generates Sub-App positions from the quad points of elements in the master mesh.");
  params.suppressParameter<std::vector<Point>>("positions");
  params.suppressParameter<std::vector<FileName>>("positions_file");
  return params;
}

QPMultiApp::QPMultiApp(const InputParameters & parameters)
  : TransientMultiApp(parameters), BlockRestrictable(this),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_fe_problem.assembly(_tid)),
    _qrule(_assembly.qRule()),
    _q_point(_assembly.qPoints())
{
}

void
QPMultiApp::fillPositions()
{
  MooseMesh & master_mesh = _fe_problem.mesh();
for (const auto & elem_ptr : master_mesh.getMesh().active_local_element_ptr_range())
{
    if (hasBlocks(elem_ptr->subdomain_id()))
{
	  _assembly.reinit(elem_ptr);
	  for (unsigned int _qp = 0; _qp < _qrule->n_points(); ++_qp)
{
	  _positions.push_back(_q_point[_qp]);
      std::cout << _q_point[_qp](0) << "," << _q_point[_qp](1) << std::endl;
}
}
}
  // Use the comm from the problem this MultiApp is part of
  libMesh::ParallelObject::comm().allgather(_positions);

  if (_positions.empty())
    mooseError("No positions found for QPMultiApp ", _name);

  // An attempt to try to make this parallel stable
  std::sort(_positions.begin(), _positions.end());
}

//  MooseMesh & master_mesh = _fe_problem.mesh();
//  MooseMesh & master_mesh = _multi_app->problemBase().mesh();
//  std::vector<Point> point_copies;
//for (const auto & elem_ptr : master_mesh.getMesh().active_local_element_ptr_range())
//{
//    if (hasBlocks(elem_ptr->subdomain_id()))
//point_copies.clear();
//point_copies.reserve(elem_ptr->n_nodes());
//{
//	  _fe_problem.setCurrentSubdomainID(elem_ptr, _tid);
//	  _fe_problem.prepare(elem_ptr, 0);
//	  _assembly.reinit(elem_ptr);
//	  for (unsigned int i = 0; i < elem_ptr->n_nodes(); ++i)
//	  for (unsigned int _qp = 0; _qp < _qrule->n_points(); ++_qp)
//{  //   _positions.push_back(elem_ptr->centroid());
// Node & displaced_node = elem_ptr->node_ref(i);
// point_copies.push_back(displaced_node);
//_positions.push_back(displaced_node);
//Point q2 = elem_ptr->centroid();
//	   _positions.push_back(_q_point[_qp]);
//std::cout << _q_point[_qp](0) << "," << _q_point[_qp](1) << std::endl;
//Point SSS = elem_ptr->centroid();	  printf ("%f\n",QQQ);
//}
//}
//}