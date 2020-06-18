//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SecondaryConstraint.h"

// Moose includes
#include "SystemBase.h"
#include "MooseMesh.h"
#include "ContactAction.h"

#include "libmesh/plane.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/string_to_enum.h"

registerMooseObject("ContactApp", SecondaryConstraint);

InputParameters
SecondaryConstraint::validParams()
{
  InputParameters params = DiracKernel::validParams();
  params += ContactAction::commonParameters();

  params.addRequiredParam<BoundaryName>("boundary", "The secondary boundary");
  params.addRequiredParam<BoundaryName>("primary", "The primary boundary");
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction "
                                        "the variable this kernel acts in. (0 for x, "
                                        "1 for y, 2 for z)");

  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");

  params.addCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");

  params.addRequiredCoupledVar("nodal_area", "The nodal area");

  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>(
      "penalty",
      1e8,
      "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<Real>("friction_coefficient", 0, "The friction coefficient");
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addParam<bool>(
      "normalize_penalty",
      false,
      "Whether to normalize the penalty parameter with the nodal area for penalty contact.");
  return params;
}

SecondaryConstraint::SecondaryConstraint(const InputParameters & parameters)
  : DiracKernel(parameters),
    _component(getParam<unsigned int>("component")),
    _model(getParam<MooseEnum>("model").getEnum<ContactModel>()),
    _formulation(getParam<MooseEnum>("formulation").getEnum<ContactFormulation>()),
    _normalize_penalty(getParam<bool>("normalize_penalty")),
    _penetration_locator(
        getPenetrationLocator(getParam<BoundaryName>("primary"),
                              getParam<BoundaryName>("boundary"),
                              Utility::string_to_enum<Order>(getParam<MooseEnum>("order")))),
    _penalty(getParam<Real>("penalty")),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _residual_copy(_sys.residualGhosted()),
    _vars(3, libMesh::invalid_uint),
    _mesh_dimension(_mesh.dimension()),
    _nodal_area_var(getVar("nodal_area", 0)),
    _aux_system(_nodal_area_var->sys()),
    _aux_solution(_aux_system.currentSolution())
{
  if (isParamValid("displacements"))
  {
    // modern parameter scheme for displacements
    for (unsigned int i = 0; i < coupledComponents("displacements"); ++i)
      _vars[i] = coupled("displacements", i);
  }
  else
  {
    // Legacy parameter scheme for displacements
    if (isParamValid("disp_x"))
      _vars[0] = coupled("disp_x");
    if (isParamValid("disp_y"))
      _vars[1] = coupled("disp_y");
    if (isParamValid("disp_z"))
      _vars[2] = coupled("disp_z");

    mooseDeprecated("use the `displacements` parameter rather than the `disp_*` parameters (those "
                    "will go away with the deprecation of the Solid Mechanics module).");
  }

  if (parameters.isParamValid("tangential_tolerance"))
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));

  if (parameters.isParamValid("normal_smoothing_distance"))
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));

  if (parameters.isParamValid("normal_smoothing_method"))
    _penetration_locator.setNormalSmoothingMethod(
        parameters.get<std::string>("normal_smoothing_method"));
}

void
SecondaryConstraint::addPoints()
{
  _point_to_info.clear();

  std::map<dof_id_type, PenetrationInfo *>::iterator
      it = _penetration_locator._penetration_info.begin(),
      end = _penetration_locator._penetration_info.end();

  const auto & node_to_elem_map = _mesh.nodeToElemMap();
  for (; it != end; ++it)
  {
    PenetrationInfo * pinfo = it->second;

    // Skip this pinfo if there are no DOFs on this node.
    if (!pinfo || pinfo->_node->n_comp(_sys.number(), _vars[_component]) < 1)
      continue;

    dof_id_type secondary_node_num = it->first;
    const Node * node = pinfo->_node;

    if (pinfo->isCaptured() && node->processor_id() == processor_id())
    {
      // Find an element that is connected to this node that and that is also on this processor
      auto node_to_elem_pair = node_to_elem_map.find(secondary_node_num);
      mooseAssert(node_to_elem_pair != node_to_elem_map.end(), "Missing node in node to elem map");
      const std::vector<dof_id_type> & connected_elems = node_to_elem_pair->second;

      Elem * elem = NULL;

      for (unsigned int i = 0; i < connected_elems.size() && !elem; ++i)
      {
        Elem * cur_elem = _mesh.elemPtr(connected_elems[i]);
        if (cur_elem->processor_id() == processor_id())
          elem = cur_elem;
      }

      mooseAssert(
          elem,
          "Couldn't find an element on this processor that is attached to the secondary node!");

      addPoint(elem, *node);
      _point_to_info[*node] = pinfo;
    }
  }
}

Real
SecondaryConstraint::computeQpResidual()
{
  PenetrationInfo * pinfo = _point_to_info[_current_point];
  const Node * node = pinfo->_node;

  Real resid = pinfo->_contact_force(_component);

  const Real area = nodalArea(*pinfo);

  if (_formulation == ContactFormulation::KINEMATIC)
  {
    RealVectorValue distance_vec(_mesh.nodeRef(node->id()) - pinfo->_closest_point);
    RealVectorValue pen_force(_penalty * distance_vec);
    if (_normalize_penalty)
      pen_force *= area;

    if (_model == ContactModel::FRICTIONLESS)
      resid += pinfo->_normal(_component) * pinfo->_normal * pen_force;

    else if (_model == ContactModel::GLUED || _model == ContactModel::COULOMB)
      resid += pen_force(_component);
  }

  return _test[_i][_qp] * resid;
}

Real
SecondaryConstraint::computeQpJacobian()
{

  // TODO: for the default formulation,
  //   we should subtract off the existing Jacobian weighted by the effect of the normal

  PenetrationInfo * pinfo = _point_to_info[_current_point];
  const Node * node = pinfo->_node;
  //   long int dof_number = node->dof_number(0, _var.number(), 0);

  //  RealVectorValue jac_vec;

  // Build up jac vector
  //  for (unsigned int i=0; i<_dim; i++)
  //  {
  //    unsigned int dof_row = _dof_data._var_dof_indices[_var_num][_i];
  //    unsigned int dof_col = _dof_data._var_dof_indices[_var_num][_j];

  //    Real jac_value = _jacobian_copy(dof_row, dof_col);
  //  }

  //    Real jac_mag = pinfo->_normal(_component) * jac_value;
  /*
     return _test[_i][_qp] * (
       (1e8*-_phi[_j][_qp])
       -_jacobian_copy(dof_number, dof_number)
       );
     */

  RealVectorValue normal(pinfo->_normal);

  Real penalty = _penalty;
  if (_normalize_penalty)
    penalty *= nodalArea(*pinfo);

  Real term(0);

  if (ContactModel::FRICTIONLESS == _model)
  {

    const Real nnTDiag = normal(_component) * normal(_component);
    term = penalty * nnTDiag;

    const RealGradient & A1(pinfo->_dxyzdxi[0]);
    RealGradient A2;
    RealGradient d2;
    if (_mesh_dimension == 3)
    {
      A2 = pinfo->_dxyzdeta[0];
      d2 = pinfo->_d2xyzdxideta[0];
    }
    else
    {
      A2.zero();
      d2.zero();
    }

    const RealVectorValue distance_vec(_mesh.nodeRef(node->id()) - pinfo->_closest_point);
    const Real ATA11(A1 * A1);
    const Real ATA12(A1 * A2);
    const Real ATA22(A2 * A2);
    const Real D11(-ATA11);
    const Real D12(-ATA12 + d2 * distance_vec);
    const Real D22(-ATA22);

    Real invD11(0);
    Real invD12(0);
    Real invD22(0);
    if (_mesh_dimension == 3)
    {
      const Real detD(D11 * D22 - D12 * D12);
      invD11 = D22 / detD;
      invD12 = -D12 / detD;
      invD22 = D11 / detD;
    }
    else if (_mesh_dimension == 2)
    {
      invD11 = 1 / D11;
    }

    const Real AinvD11(A1(0) * invD11 + A2(0) * invD12);
    const Real AinvD12(A1(0) * invD12 + A2(0) * invD22);
    const Real AinvD21(A1(1) * invD11 + A2(1) * invD12);
    const Real AinvD22(A1(1) * invD12 + A2(1) * invD22);
    const Real AinvD31(A1(2) * invD11 + A2(2) * invD12);
    const Real AinvD32(A1(2) * invD12 + A2(2) * invD22);

    const Real AinvDAT11(AinvD11 * A1(0) + AinvD12 * A2(0));
    //     const Real AinvDAT12( AinvD11*A1(1) + AinvD12*A2(1) );
    //     const Real AinvDAT13( AinvD11*A1(2) + AinvD12*A2(2) );
    //     const Real AinvDAT21( AinvD21*A1(0) + AinvD22*A2(0) );
    const Real AinvDAT22(AinvD21 * A1(1) + AinvD22 * A2(1));
    //     const Real AinvDAT23( AinvD21*A1(2) + AinvD22*A2(2) );
    //     const Real AinvDAT31( AinvD31*A1(0) + AinvD32*A2(0) );
    //     const Real AinvDAT32( AinvD31*A1(1) + AinvD32*A2(1) );
    const Real AinvDAT33(AinvD31 * A1(2) + AinvD32 * A2(2));

    if (_component == 0)
      term += penalty * (1 - nnTDiag + AinvDAT11);

    else if (_component == 1)
      term += penalty * (1 - nnTDiag + AinvDAT22);

    else
      term += penalty * (1 - nnTDiag + AinvDAT33);
  }
  else if (ContactModel::GLUED == _model || ContactModel::COULOMB == _model)
  {
    normal.zero();
    normal(_component) = 1;
    term = penalty;
  }
  else
  {
    mooseError("Invalid or unavailable contact model");
  }

  return _test[_i][_qp] * term * _phi[_j][_qp];
}

Real
SecondaryConstraint::nodalArea(PenetrationInfo & pinfo)
{
  const Node * node = pinfo._node;

  dof_id_type dof = node->dof_number(_aux_system.number(), _nodal_area_var->number(), 0);

  Real area = (*_aux_solution)(dof);
  if (area == 0)
  {
    if (_t_step > 1)
      mooseError("Zero nodal area found");

    else
      area = 1; // Avoid divide by zero during initialization
  }
  return area;
}
