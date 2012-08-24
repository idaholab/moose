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
  params.addRequiredParam<VariableName>("variable", "The name of the variable that this side indicator applies to");
  params.addParam<bool>("scale_by_flux_faces", false, "Whether or not to scale the error values by the number of flux faces.  This attempts to not penalize elements on boundaries for having less neighbors.");

  params.addPrivateParam<BoundaryID>("_boundary_id", InternalSideIndicator::InternalBndId);

  params.addPrivateParam<std::string>("built_by_action", "add_indicator");
  return params;
}


InternalSideIndicator::InternalSideIndicator(const std::string & name, InputParameters parameters) :
    Indicator(name, parameters),
    NeighborCoupleable(parameters, false),
    NeighborMooseVariableInterface(parameters, false),
    TwoMaterialPropertyInterface(parameters),

    _field_var(_sys.getVariable(_tid, name)),

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

    _var(_subproblem.getVariable(_tid, parameters.get<VariableName>("variable"))),
    _scale_by_flux_faces(parameters.get<bool>("scale_by_flux_faces")),

    _u(_var.sln()),
    _grad_u(_var.gradSln()),

    _normals(_field_var.normals()),

    _u_neighbor(_var.slnNeighbor()),
    _grad_u_neighbor(_var.gradSlnNeighbor())
{
}

InternalSideIndicator::~InternalSideIndicator()
{
}

void
InternalSideIndicator::computeIndicator()
{
  Real sum = 0;

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    sum += _JxW[_qp]*_coord[_qp]*computeQpIntegral();

  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

    _solution.add(_field_var.nodalDofIndex(), sum*_current_elem->hmax());
    _solution.add(_field_var.nodalDofIndexNeighbor(), sum*_neighbor_elem->hmax());
  }
}

void
InternalSideIndicator::finalize()
{
  unsigned int n_flux_faces = 0;

  if(_scale_by_flux_faces)
  {
    // Figure out the total number of sides contributing to the error.
    // We'll scale by this so boundary elements are less penalized
    for (unsigned int side=0; side<_current_elem->n_sides(); side++)
      if(_current_elem->neighbor(side) != NULL)
        n_flux_faces++;
  }
  else
    n_flux_faces = 1;

  // The 0 is because CONSTANT MONOMIALS only have one coefficient per element...
  Real value = _field_var.nodalSln()[0];

  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _solution.set(_field_var.nodalDofIndex(), std::sqrt(value)/(Real)n_flux_faces);
  }
}
