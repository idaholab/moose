//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "PenetrationAux.h"
#include "PenetrationLocator.h"
#include "DisplacedProblem.h"
#include "MooseEnum.h"
#include "MooseMesh.h"

#include "libmesh/string_to_enum.h"

registerMooseObject("MooseApp", PenetrationAux);

InputParameters
PenetrationAux::validParams()
{
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Auxiliary Kernel for computing several geometry related quantities "
                             "between two contacting bodies.");

  params.addRequiredParam<BoundaryName>("paired_boundary", "The boundary to be penetrated");
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>(
      "normal_smoothing_distance",
      "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method",
                               "Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addCoupledVar("secondary_gap_offset", "offset to the gap distance from secondary side");
  params.addCoupledVar("mapped_primary_gap_offset",
                       "offset to the gap distance mapped from primary side");

  params.set<bool>("use_displaced_mesh") = true;

  // To avoid creating a conversion routine we will list the enumeration options in the same order
  // as the class-based enum.
  // Care must be taken to ensure that this list stays in sync with the enum in the .h file.
  MooseEnum quantity(
      "distance tangential_distance normal_x normal_y normal_z closest_point_x closest_point_y "
      "closest_point_z element_id side incremental_slip_magnitude incremental_slip_x "
      "incremental_slip_y incremental_slip_z accumulated_slip force_x force_y force_z "
      "normal_force_magnitude normal_force_x normal_force_y normal_force_z "
      "tangential_force_magnitude "
      "tangential_force_x tangential_force_y tangential_force_z frictional_energy "
      "lagrange_multiplier "
      "mechanical_status",
      "distance");

  params.addParam<MooseEnum>(
      "quantity", quantity, "The quantity to recover from the available penetration information");
  return params;
}

PenetrationAux::PenetrationAux(const InputParameters & parameters)
  : AuxKernel(parameters),

    // Here we cast the value of the MOOSE enum to an integer to the class-based enum.
    _quantity(getParam<MooseEnum>("quantity").getEnum<PenetrationAux::PA_ENUM>()),
    _penetration_locator(
        _nodal ? getPenetrationLocator(
                     parameters.get<BoundaryName>("paired_boundary"),
                     boundaryNames()[0],
                     Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")))
               : getQuadraturePenetrationLocator(
                     parameters.get<BoundaryName>("paired_boundary"),
                     boundaryNames()[0],
                     Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")))),
    _has_secondary_gap_offset(isCoupled("secondary_gap_offset")),
    _secondary_gap_offset_var(_has_secondary_gap_offset ? getVar("secondary_gap_offset", 0)
                                                        : nullptr),
    _has_mapped_primary_gap_offset(isCoupled("mapped_primary_gap_offset")),
    _mapped_primary_gap_offset_var(
        _has_mapped_primary_gap_offset ? getVar("mapped_primary_gap_offset", 0) : nullptr)
{
  if (parameters.isParamValid("tangential_tolerance"))
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));

  if (parameters.isParamValid("normal_smoothing_distance"))
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));

  if (parameters.isParamValid("normal_smoothing_method"))
    _penetration_locator.setNormalSmoothingMethod(
        parameters.get<std::string>("normal_smoothing_method"));
}

Real
PenetrationAux::computeValue()
{
  const Node * current_node = nullptr;

  if (_nodal)
    current_node = _current_node;
  else
    current_node = _mesh.getQuadratureNode(_current_elem, _current_side, _qp);

  PenetrationInfo * pinfo = _penetration_locator._penetration_info[current_node->id()];

  // A node that doesn't project has zero force, closest point (i.e. not computed), slip, and its
  // mechanical status is not in contact.
  Real retVal(0);

  if (pinfo)
    switch (_quantity)
    {
      case PA_DISTANCE:
        retVal =
            pinfo->_distance -
            (_has_secondary_gap_offset ? _secondary_gap_offset_var->getNodalValue(*current_node)
                                       : 0) -
            (_has_mapped_primary_gap_offset
                 ? _mapped_primary_gap_offset_var->getNodalValue(*current_node)
                 : 0);
        break;
      case PA_TANG_DISTANCE:
        retVal = pinfo->_tangential_distance;
        break;
      case PA_NORMAL_X:
        retVal = pinfo->_normal(0);
        break;
      case PA_NORMAL_Y:
        retVal = pinfo->_normal(1);
        break;
      case PA_NORMAL_Z:
        retVal = pinfo->_normal(2);
        break;
      case PA_CLOSEST_POINT_X:
        retVal = pinfo->_closest_point(0);
        break;
      case PA_CLOSEST_POINT_Y:
        retVal = pinfo->_closest_point(1);
        break;
      case PA_CLOSEST_POINT_Z:
        retVal = pinfo->_closest_point(2);
        break;
      case PA_ELEM_ID:
        retVal = static_cast<Real>(pinfo->_elem->id() + 1);
        break;
      case PA_SIDE:
        retVal = static_cast<Real>(pinfo->_side_num);
        break;
      case PA_INCREMENTAL_SLIP_MAG:
        retVal = pinfo->isCaptured() ? pinfo->_incremental_slip.norm() : 0;
        break;
      case PA_INCREMENTAL_SLIP_X:
        retVal = pinfo->isCaptured() ? pinfo->_incremental_slip(0) : 0;
        break;
      case PA_INCREMENTAL_SLIP_Y:
        retVal = pinfo->isCaptured() ? pinfo->_incremental_slip(1) : 0;
        break;
      case PA_INCREMENTAL_SLIP_Z:
        retVal = pinfo->isCaptured() ? pinfo->_incremental_slip(2) : 0;
        break;
      case PA_ACCUMULATED_SLIP:
        retVal = pinfo->_accumulated_slip;
        break;
      case PA_FORCE_X:
        retVal = pinfo->_contact_force(0);
        break;
      case PA_FORCE_Y:
        retVal = pinfo->_contact_force(1);
        break;
      case PA_FORCE_Z:
        retVal = pinfo->_contact_force(2);
        break;
      case PA_NORMAL_FORCE_MAG:
        retVal = -pinfo->_contact_force * pinfo->_normal;
        break;
      case PA_NORMAL_FORCE_X:
        retVal = (pinfo->_contact_force * pinfo->_normal) * pinfo->_normal(0);
        break;
      case PA_NORMAL_FORCE_Y:
        retVal = (pinfo->_contact_force * pinfo->_normal) * pinfo->_normal(1);
        break;
      case PA_NORMAL_FORCE_Z:
        retVal = (pinfo->_contact_force * pinfo->_normal) * pinfo->_normal(2);
        break;
      case PA_TANGENTIAL_FORCE_MAG:
      {
        RealVectorValue contact_force_normal((pinfo->_contact_force * pinfo->_normal) *
                                             pinfo->_normal);
        RealVectorValue contact_force_tangential(pinfo->_contact_force - contact_force_normal);
        retVal = contact_force_tangential.norm();
        break;
      }
      case PA_TANGENTIAL_FORCE_X:
        retVal =
            pinfo->_contact_force(0) - (pinfo->_contact_force * pinfo->_normal) * pinfo->_normal(0);
        break;
      case PA_TANGENTIAL_FORCE_Y:
        retVal =
            pinfo->_contact_force(1) - (pinfo->_contact_force * pinfo->_normal) * pinfo->_normal(1);
        break;
      case PA_TANGENTIAL_FORCE_Z:
        retVal =
            pinfo->_contact_force(2) - (pinfo->_contact_force * pinfo->_normal) * pinfo->_normal(2);
        break;
      case PA_FRICTIONAL_ENERGY:
        retVal = pinfo->_frictional_energy;
        break;
      case PA_LAGRANGE_MULTIPLIER:
        retVal = pinfo->_lagrange_multiplier;
        break;
      case PA_MECH_STATUS:
        retVal = pinfo->_mech_status;
        break;
      default:
        mooseError("Unknown penetration info quantity in auxiliary kernel.");
    } // switch

  return retVal;
}
