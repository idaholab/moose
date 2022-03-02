//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NodalNormalsPreprocessor.h"

#include "Assembly.h"
#include "AuxiliarySystem.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/quadrature.h"

std::mutex NodalNormalsPreprocessor::_nodal_normals_mutex;

registerMooseObject("MooseApp", NodalNormalsPreprocessor);

InputParameters
NodalNormalsPreprocessor::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription(
      "An object that prepares MOOSE for computing nodal normal vectors. This object is "
      "automatically created via the \\[NodalNormals\\] input block.");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "surface_boundary", "The list of boundary IDs where nodal normals are computed");
  params.addParam<BoundaryName>("corner_boundary",
                                "Node set ID which contains the nodes that are in 'corners'.");
  params.addPrivateParam<FEFamily>("fe_family", LAGRANGE);
  params.addPrivateParam<Order>("fe_order", FIRST);

  return params;
}

/**
 * Local function to check to see if any intersection occurs between two vectors. Neither can be
 * assumed to be sorted but both are generally very short (just 1 or 2 entries) so doing an explicit
 * double loop is probably the easiest.
 */
bool
hasBoundary(const std::vector<BoundaryID> & boundary_ids1,
            const std::vector<BoundaryID> & boundary_ids2)
{
  for (auto id1 : boundary_ids1)
  {
    if (id1 == Moose::ANY_BOUNDARY_ID)
      return true;

    for (auto id2 : boundary_ids2)
      if (id1 == id2 || id2 == Moose::ANY_BOUNDARY_ID)
        return true;
  }
  return false;
}

NodalNormalsPreprocessor::NodalNormalsPreprocessor(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _aux(_fe_problem.getAuxiliarySystem()),
    _fe_type(getParam<Order>("fe_order"), getParam<FEFamily>("fe_family")),
    _has_corners(isParamValid("corner_boundary")),
    _boundaries(_mesh.getBoundaryIDs(getParam<std::vector<BoundaryName>>("surface_boundary"))),
    _corner_boundary_id(_has_corners
                            ? _mesh.getBoundaryID(getParam<BoundaryName>("corner_boundary"))
                            : static_cast<BoundaryID>(-1)),
    _grad_phi(_assembly.feGradPhi<Real>(_fe_type))
{
}

void
NodalNormalsPreprocessor::initialize()
{
  NumericVector<Number> & sln = _aux.solution();
  _aux.system().zero_variable(sln, _aux.getVariable(_tid, "nodal_normal_x").number());
  _aux.system().zero_variable(sln, _aux.getVariable(_tid, "nodal_normal_y").number());
  _aux.system().zero_variable(sln, _aux.getVariable(_tid, "nodal_normal_z").number());
  // After zero variables, we should close the solution
  sln.close();
}

void
NodalNormalsPreprocessor::execute()
{
  NumericVector<Number> & sln = _aux.solution();

  // Get a reference to our BoundaryInfo object for later use...
  BoundaryInfo & boundary_info = _mesh.getMesh().get_boundary_info();

  // Container to catch IDs handed back by BoundaryInfo.
  std::vector<BoundaryID> node_boundary_ids;

  // Loop through each node on the current element
  for (unsigned int i = 0; i < _current_elem->n_nodes(); i++)
  {
    // Extract a pointer to a node
    const Node * node = _current_elem->node_ptr(i);

    // Only continue if the node is on a boundary
    if (_mesh.isBoundaryNode(node->id()))
    {
      // List of IDs for the boundary
      boundary_info.boundary_ids(node, node_boundary_ids);

      // Perform the calculation, the node must be:
      //    (1) On a boundary to which the object is restricted
      //    (2) Not on a corner of the boundary
      if (hasBoundary(node_boundary_ids, _boundaries) &&
          (!_has_corners || !boundary_info.has_boundary_id(node, _corner_boundary_id)))
      {
        // Perform the caluation of the normal
        if (node->n_dofs(_aux.number(),
                         _fe_problem
                             .getVariable(_tid,
                                          "nodal_normal_x",
                                          Moose::VarKindType::VAR_AUXILIARY,
                                          Moose::VarFieldType::VAR_FIELD_STANDARD)
                             .number()) > 0)
        {
          // but it is not a corner node, they will be treated differently later on
          dof_id_type dof_x =
              node->dof_number(_aux.number(),
                               _fe_problem
                                   .getVariable(_tid,
                                                "nodal_normal_x",
                                                Moose::VarKindType::VAR_AUXILIARY,
                                                Moose::VarFieldType::VAR_FIELD_STANDARD)
                                   .number(),
                               0);
          dof_id_type dof_y =
              node->dof_number(_aux.number(),
                               _fe_problem
                                   .getVariable(_tid,
                                                "nodal_normal_y",
                                                Moose::VarKindType::VAR_AUXILIARY,
                                                Moose::VarFieldType::VAR_FIELD_STANDARD)
                                   .number(),
                               0);
          dof_id_type dof_z =
              node->dof_number(_aux.number(),
                               _fe_problem
                                   .getVariable(_tid,
                                                "nodal_normal_z",
                                                Moose::VarKindType::VAR_AUXILIARY,
                                                Moose::VarFieldType::VAR_FIELD_STANDARD)
                                   .number(),
                               0);

          for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
          {
            std::scoped_lock lock(_nodal_normals_mutex);

            sln.add(dof_x, _JxW[qp] * _grad_phi[i][qp](0));
            sln.add(dof_y, _JxW[qp] * _grad_phi[i][qp](1));
            sln.add(dof_z, _JxW[qp] * _grad_phi[i][qp](2));
          }
        }
      }
    }
  }
}

void
NodalNormalsPreprocessor::finalize()
{
  _aux.solution().close();
}

void
NodalNormalsPreprocessor::threadJoin(const UserObject & /*uo*/)
{
}
