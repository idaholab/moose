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

#include "PenetrationAux.h"

#include "DisplacedProblem.h"
#include "MooseEnum.h"

#include "libmesh/string_to_enum.h"

const Real PenetrationAux::NotPenetrated = -999999;

template<>
InputParameters validParams<PenetrationAux>()
{
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<BoundaryName>("paired_boundary", "The boundary to be penetrated");
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>("normal_smoothing_distance", "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method","Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addParam<bool>("added_by_contact_action", false, "True if this kernel was added by ContactAction (internal use only)");

  params.set<bool>("use_displaced_mesh") = true;

  // To avoid creating a conversion routine we will list the enumeration options in the same order as the class-based enum.
  // Care must be taken to ensure that this list stays in sync with the enum in the .h file.
  MooseEnum quantity("distance tangential_distance normal_x normal_y normal_z closest_point_x closest_point_y closest_point_z "
                     "element_id side incremental_slip_magnitude incremental_slip_x incremental_slip_y incremental_slip_z accumulated_slip "
                     "force_x force_y force_z normal_force_magnitude normal_force_x normal_force_y normal_force_z tangential_force_magnitude "
                     "tangential_force_x tangential_force_y tangential_force_z frictional_energy mechanical_status", "distance");

  params.addParam<MooseEnum>("quantity", quantity, "The quantity to recover from the available penetration information");
  return params;
}

PenetrationAux::PenetrationAux(const InputParameters & parameters) :
    AuxKernel(parameters),

    // Here we cast the value of the MOOSE enum to an integer to the class-based enum.
    _quantity(PenetrationAux::PA_ENUM(int(getParam<MooseEnum>("quantity")))),
    _penetration_locator(_nodal
      ? getPenetrationLocator(
          parameters.get<BoundaryName>("paired_boundary"),
          boundaryNames()[0],
          Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")))
      : getQuadraturePenetrationLocator(
          parameters.get<BoundaryName>("paired_boundary"),
          boundaryNames()[0],
          Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")))),
    _should_update_old_state(getParam<bool>("added_by_contact_action"))
{
  if ( _should_update_old_state )
    mooseAssert( dynamic_cast< DisplacedProblem * >( &_subproblem ), "use_displaced_mesh must be true if this kernel is added by ContactAction" );

  if (parameters.isParamValid("tangential_tolerance"))
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));

  if (parameters.isParamValid("normal_smoothing_distance"))
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));

  if (parameters.isParamValid("normal_smoothing_method"))
    _penetration_locator.setNormalSmoothingMethod(parameters.get<std::string>("normal_smoothing_method"));
}

PenetrationAux::~PenetrationAux()
{
}

void
PenetrationAux::timestepSetup()
{
  AuxKernel::timestepSetup();

  // Update PenetrationInfo old states using the stored aux variables in oldSolution.
  if ( _should_update_old_state )
  {
    DisplacedProblem & displaced_problem = dynamic_cast< DisplacedProblem & >( _subproblem );

    // All nodes (ghosted included) should get updated copies of the old contact force,
    // accumulated slip, and frictional energy.
    typedef std::map<dof_id_type, PenetrationInfo *> PenetrationMap;
    PenetrationMap::iterator begin = _penetration_locator._penetration_info.begin();
    PenetrationMap::iterator end = _penetration_locator._penetration_info.end();
    for ( PenetrationMap::iterator it = begin; it != end; ++it )
    {
      PenetrationInfo * pinfo = it->second;
      if ( pinfo )
      {
        // We have to re-get the node here because getNodalValueOld asserts isSemiLocal based on a
        // pointer to the node rather than the node's ID. So we need a reference to the actual node
        // held by the mesh rather than the displaced copy of the node stored in the pinfo.
        Node & node = displaced_problem.refMesh().node( pinfo->_node->id() );
        Real old_value = _var.getNodalValueOld(node);

        switch (_quantity)
        {
          case PA_FORCE_X:
            pinfo->_contact_force_old(0) = old_value;
            pinfo->_contact_force(0) = pinfo->_contact_force_old(0);
            break;
          case PA_FORCE_Y:
            pinfo->_contact_force_old(1) = old_value;
            pinfo->_contact_force(1) = pinfo->_contact_force_old(1);
            break;
          case PA_FORCE_Z:
            pinfo->_contact_force_old(2) = old_value;
            pinfo->_contact_force(2) = pinfo->_contact_force_old(2);
            break;
          case PA_ACCUMULATED_SLIP:
            pinfo->_accumulated_slip_old = old_value;
            pinfo->_accumulated_slip = pinfo->_accumulated_slip_old;
            break;
          case PA_FRICTIONAL_ENERGY:
            pinfo->_frictional_energy_old = old_value;
            pinfo->_frictional_energy = pinfo->_frictional_energy_old;
            break;
          default:
            ; // Suppress the warning for unhandled cases.
        }
      }
    }
  }
}

Real
PenetrationAux::computeValue()
{
  const Node * current_node = NULL;

  if (_nodal)
    current_node = _current_node;
  else
    current_node = _mesh.getQuadratureNode(_current_elem, _current_side, _qp);

  PenetrationInfo * pinfo = _penetration_locator._penetration_info[current_node->id()];

  Real retVal(NotPenetrated);
  if ( _should_update_old_state )
    retVal = 0;

  if (pinfo)
    switch (_quantity)
    {
      case PA_DISTANCE:
        retVal = pinfo->_distance; break;
      case PA_TANG_DISTANCE:
        retVal = pinfo->_tangential_distance; break;
      case PA_NORMAL_X:
        retVal = pinfo->_normal(0); break;
      case PA_NORMAL_Y:
        retVal = pinfo->_normal(1); break;
      case PA_NORMAL_Z:
        retVal = pinfo->_normal(2); break;
      case PA_CLOSEST_POINT_X:
        retVal = pinfo->_closest_point(0); break;
      case PA_CLOSEST_POINT_Y:
        retVal = pinfo->_closest_point(1); break;
      case PA_CLOSEST_POINT_Z:
        retVal = pinfo->_closest_point(2); break;
      case PA_ELEM_ID:
        retVal = (Real) (pinfo->_elem->id()+1); break;
      case PA_SIDE:
        retVal = (Real) (pinfo->_side_num); break;
      case PA_INCREMENTAL_SLIP_MAG:
      {
        if (pinfo->_mech_status == PenetrationInfo::MS_NO_CONTACT)
          retVal = 0.0;
        else
          retVal = pinfo->_incremental_slip.size();
        break;
      }
      case PA_INCREMENTAL_SLIP_X:
      {
        if (pinfo->_mech_status == PenetrationInfo::MS_NO_CONTACT)
          retVal = 0.0;
        else
          retVal = pinfo->_incremental_slip(0);
        break;
      }
      case PA_INCREMENTAL_SLIP_Y:
      {
        if (pinfo->_mech_status == PenetrationInfo::MS_NO_CONTACT)
          retVal = 0.0;
        else
          retVal = pinfo->_incremental_slip(1);
        break;
      }
      case PA_INCREMENTAL_SLIP_Z:
      {
        if (pinfo->_mech_status == PenetrationInfo::MS_NO_CONTACT)
          retVal = 0.0;
        else
          retVal = pinfo->_incremental_slip(2);
        break;
      }
      case PA_ACCUMULATED_SLIP:
        retVal = pinfo->_accumulated_slip; break;
      case PA_FORCE_X:
        retVal = pinfo->_contact_force(0); break;
      case PA_FORCE_Y:
        retVal = pinfo->_contact_force(1); break;
      case PA_FORCE_Z:
        retVal = pinfo->_contact_force(2); break;
      case PA_NORMAL_FORCE_MAG:
        retVal = -pinfo->_contact_force*pinfo->_normal; break;
      case PA_NORMAL_FORCE_X:
        retVal = (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal(0); break;
      case PA_NORMAL_FORCE_Y:
        retVal = (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal(1); break;
      case PA_NORMAL_FORCE_Z:
        retVal = (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal(2); break;
      case PA_TANGENTIAL_FORCE_MAG:
      {
        RealVectorValue contact_force_normal( (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal );
        RealVectorValue contact_force_tangential( pinfo->_contact_force - contact_force_normal );
        retVal = contact_force_tangential.size();
        break;
      }
      case PA_TANGENTIAL_FORCE_X:
        retVal = pinfo->_contact_force(0) - (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal(0); break;
      case PA_TANGENTIAL_FORCE_Y:
        retVal = pinfo->_contact_force(1) - (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal(1); break;
      case PA_TANGENTIAL_FORCE_Z:
        retVal = pinfo->_contact_force(2) - (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal(2); break;
      case PA_FRICTIONAL_ENERGY:
        retVal = pinfo->_frictional_energy; break;
      case PA_MECH_STATUS:
        retVal = pinfo->_mech_status; break;
      default:
        mooseError("Unknown PA_ENUM");
    } // switch

  return retVal;
}


// DEPRECATED CONSTRUCTOR
PenetrationAux::PenetrationAux(const std::string & deprecated_name, InputParameters parameters) :
    AuxKernel(deprecated_name, parameters),

    // Here we cast the value of the MOOSE enum to an integer to the class-based enum.
    _quantity(PenetrationAux::PA_ENUM(int(getParam<MooseEnum>("quantity")))),
    _penetration_locator(_nodal
      ? getPenetrationLocator(
          parameters.get<BoundaryName>("paired_boundary"),
          boundaryNames()[0],
          Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")))
      : getQuadraturePenetrationLocator(
          parameters.get<BoundaryName>("paired_boundary"),
          boundaryNames()[0],
          Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")))),
    _should_update_old_state(getParam<bool>("added_by_contact_action"))
{
  if ( _should_update_old_state )
    mooseAssert( dynamic_cast< DisplacedProblem * >( &_subproblem ), "use_displaced_mesh must be true if this kernel is added by ContactAction" );

  if (parameters.isParamValid("tangential_tolerance"))
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));

  if (parameters.isParamValid("normal_smoothing_distance"))
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));

  if (parameters.isParamValid("normal_smoothing_method"))
    _penetration_locator.setNormalSmoothingMethod(parameters.get<std::string>("normal_smoothing_method"));
}
