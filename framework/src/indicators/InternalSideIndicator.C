/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "InternalSideIndicator.h"
#include "Moose.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MaterialData.h"
#include "ParallelUniqueId.h"

// libMesh includes
#include "dof_map.h"
#include "dense_vector.h"
#include "numeric_vector.h"
#include "dense_subvector.h"
#include "libmesh_common.h"

const BoundaryID InternalSideIndicator::InternalBndId = 12345;

template<>
InputParameters validParams<InternalSideIndicator>()
{
  InputParameters params = validParams<Indicator>();
  // params.addRequiredParam<std::string>("variable", "The name of the variable that this side indicator applies to");
  params.addPrivateParam<BoundaryID>("_boundary_id", InternalSideIndicator::InternalBndId);

//  params.addPrivateParam<std::string>("built_by_action", "add_side_indicator");
  return params;
}


InternalSideIndicator::InternalSideIndicator(const std::string & name, InputParameters parameters) :
    Indicator(name, parameters),
    NeighborCoupleable(parameters, false),
    NeighborMooseVariableInterface(parameters, false),
    TwoMaterialPropertyInterface(parameters),

    _current_elem(_assembly.elem()),
    _neighbor_elem(_assembly.neighbor()),

    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),

    _coord_sys(_subproblem.coordSystem(_tid)),
    _q_point(_subproblem.pointsFace(_tid)),
    _qrule(_subproblem.qRuleFace(_tid)),
    _JxW(_subproblem.JxWFace(_tid)),
    _coord(_subproblem.coords(_tid)),

    _boundary_id(parameters.get<BoundaryID>("_boundary_id")),

    _u(_field_var.sln()),
    _grad_u(_field_var.gradSln()),

    _normals(_field_var.normals()),

    _u_neighbor(_field_var.slnNeighbor()),
    _grad_u_neighbor(_field_var.gradSlnNeighbor())
{
}

InternalSideIndicator::~InternalSideIndicator()
{
}

void
InternalSideIndicator::computeIndicator()
{
  Moose::perf_log.push("computeIndicator()","InternalSideIndicator");
  /*
  DenseVector<Number> & re = _assembly.residualBlock(_field_var.number());
  DenseVector<Number> & neighbor_re = _assembly.residualBlockNeighbor(_field_var.number());

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    for (_i=0; _i<_test.size(); _i++)
      re(_i) += _JxW[_qp]*_coord[_qp]*computeQpResidual(Moose::Element);

    for (_i=0; _i<_test_neighbor.size(); _i++)
      neighbor_re(_i) += _JxW[_qp]*_coord[_qp]*computeQpResidual(Moose::Neighbor);
  }
  */
  Moose::perf_log.pop("computeIndicator()","InternalSideIndicator");
}


