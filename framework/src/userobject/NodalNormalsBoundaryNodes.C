//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalNormalsBoundaryNodes.h"
#include "NodalNormalsUserObject.h"
#include "Assembly.h"
#include "MooseMesh.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/quadrature.h"

Threads::spin_mutex nodal_normals_preprocessor_mutex;

template <>
InputParameters
validParams<NodalNormalsBoundaryNodes>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addParam<BoundaryName>("corner_boundary",
                                "Node set ID which contains the nodes that are in 'corners'.");
  params.addPrivateParam<FEFamily>("fe_family", LAGRANGE);
  params.addPrivateParam<Order>("fe_order", FIRST);
  params.addRequiredParam<UserObjectName>(
      "nodal_normals_uo", "The name of the user object that holds the nodal normals");

  return params;
}

NodalNormalsBoundaryNodes::NodalNormalsBoundaryNodes(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _fe_type(getParam<Order>("fe_order"), getParam<FEFamily>("fe_family")),
    _has_corners(isParamValid("corner_boundary")),
    _corner_boundary_id(_has_corners
                            ? _mesh.getBoundaryID(getParam<BoundaryName>("corner_boundary"))
                            : static_cast<BoundaryID>(-1)),
    _grad_phi(_assembly.feGradPhi(_fe_type)),
    _nodal_normals_uo(getUserObject<NodalNormalsUserObject>("nodal_normals_uo"))
{
}

void
NodalNormalsBoundaryNodes::initialize()
{
  _nodal_normals_uo.zeroNormals();
}

void
NodalNormalsBoundaryNodes::execute()
{
  BoundaryInfo & boundary_info = _mesh.getMesh().get_boundary_info();
  std::vector<BoundaryID> node_boundary_ids;

  // Need to honor the type of the FE variable associated with the normals computation. A loop
  // over nodes does not work, becuase 1st order variables have less DoFs then there is nodes on
  // higher order meshes. However, this works becuase the local numbering of higher order DoFs
  // starts the same as the lower order numbering.
  for (unsigned int i = 0; i < _grad_phi.size(); i++)
  {
    const Node * node = _current_elem->node_ptr(i);
    if (_mesh.isBoundaryNode(node->id()) &&
        !(_has_corners && boundary_info.has_boundary_id(node, _corner_boundary_id)))
    {
      RealGradient grad(0, 0, 0);
      for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
        grad += _JxW[qp] * _grad_phi[i][qp];
      _nodal_normals_uo.add(node, grad);
    }
  }
}

void
NodalNormalsBoundaryNodes::finalize()
{
  _nodal_normals_uo.communicate();
  _nodal_normals_uo.computeNormals();
}

void
NodalNormalsBoundaryNodes::threadJoin(const UserObject & /*uo*/)
{
}
