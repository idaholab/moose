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
#include "MooseEnum.h"

#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<PenetrationAux>()
{
  MooseEnum orders("FIRST, SECOND, THIRD, FOURTH", "FIRST");

  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<BoundaryName>("paired_boundary", "The boundary to be penetrated");
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>("normal_smoothing_distance", "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<std::string>("quantity","distance","The quantity to recover from the available penetration information: distance(default), tangential_distance, normal_x, normal_y, normal_z, closest_point_x, closest_point_y, closest_point_z, element_id, side, incremental_slip_x, incremental_slip_y, incremental_slip_z, incremental_slip_magnitude, accumulated_slip, force_x, force_y, force_z, normal_force_magnitude, normal_force_x, normal_force_y, normal_force_z, tangential_force_magnitude, tangential_force_x, tangential_force_y, tangential_force_z, frictional_energy, mechanical_status");
  return params;
}

PenetrationAux::PenetrationAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _quantity_string( getParam<std::string>("quantity") ),
    _quantity(PA_DISTANCE),
    _penetration_locator(_nodal ?  getPenetrationLocator(parameters.get<BoundaryName>("paired_boundary"), getParam<std::vector<BoundaryName> >("boundary")[0], Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order"))) : getQuadraturePenetrationLocator(parameters.get<BoundaryName>("paired_boundary"), getParam<std::vector<BoundaryName> >("boundary")[0], Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order"))))
{
  if (parameters.isParamValid("tangential_tolerance"))
  {
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));
  }
  if (parameters.isParamValid("normal_smoothing_distance"))
  {
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));
  }

  if ( _quantity_string == "distance" )
    _quantity = PA_DISTANCE;
  else if ( _quantity_string == "tangential_distance" )
    _quantity = PA_TANG_DISTANCE;
  else if ( _quantity_string == "normal_x" )
    _quantity = PA_NORMAL_X;
  else if ( _quantity_string == "normal_y" )
    _quantity = PA_NORMAL_Y;
  else if ( _quantity_string == "normal_z" )
    _quantity = PA_NORMAL_Z;
  else if ( _quantity_string == "closest_point_x" )
    _quantity = PA_CLOSEST_POINT_X;
  else if ( _quantity_string == "closest_point_y" )
    _quantity = PA_CLOSEST_POINT_Y;
  else if ( _quantity_string == "closest_point_z" )
    _quantity = PA_CLOSEST_POINT_Z;
  else if ( _quantity_string == "element_id" )
    _quantity = PA_ELEM_ID;
  else if ( _quantity_string == "side" )
    _quantity = PA_SIDE;
  else if ( _quantity_string == "incremental_slip_magnitude" )
    _quantity = PA_INCREMENTAL_SLIP_MAG;
  else if ( _quantity_string == "incremental_slip_x" )
    _quantity = PA_INCREMENTAL_SLIP_X;
  else if ( _quantity_string == "incremental_slip_y" )
    _quantity = PA_INCREMENTAL_SLIP_Y;
  else if ( _quantity_string == "incremental_slip_z" )
    _quantity = PA_INCREMENTAL_SLIP_Z;
  else if ( _quantity_string == "accumulated_slip" )
    _quantity = PA_ACCUMULATED_SLIP;
  else if ( _quantity_string == "force_x" )
    _quantity = PA_FORCE_X;
  else if ( _quantity_string == "force_y" )
    _quantity = PA_FORCE_Y;
  else if ( _quantity_string == "force_z" )
    _quantity = PA_FORCE_Z;
  else if ( _quantity_string == "normal_force_magnitude" )
    _quantity = PA_NORMAL_FORCE_MAG;
  else if ( _quantity_string == "normal_force_x" )
    _quantity = PA_NORMAL_FORCE_X;
  else if ( _quantity_string == "normal_force_y" )
    _quantity = PA_NORMAL_FORCE_Y;
  else if ( _quantity_string == "normal_force_z" )
    _quantity = PA_NORMAL_FORCE_Z;
  else if ( _quantity_string == "tangential_force_magnitude" )
    _quantity = PA_TANGENTIAL_FORCE_MAG;
  else if ( _quantity_string == "tangential_force_x" )
    _quantity = PA_TANGENTIAL_FORCE_X;
  else if ( _quantity_string == "tangential_force_y" )
    _quantity = PA_TANGENTIAL_FORCE_Y;
  else if ( _quantity_string == "tangential_force_z" )
    _quantity = PA_TANGENTIAL_FORCE_Z;
  else if ( _quantity_string == "frictional_energy" )
    _quantity = PA_FRICTIONAL_ENERGY;
  else if ( _quantity_string == "mechanical_status" )
    _quantity = PA_MECH_STATUS;
  else
    mooseError("Invalid quantity type in PenetrationAux: "<<_quantity_string);
}

PenetrationAux::~PenetrationAux()
{
}

Real
PenetrationAux::computeValue()
{
  const Node * current_node = NULL;

  if(_nodal)
    current_node = _current_node;
  else
    current_node = _mesh.getQuadratureNode(_current_elem, _current_side, _qp);

  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[current_node->id()];

  Real retVal(-999999);
  if(pinfo)
  {
    if (_quantity == PA_DISTANCE)
      retVal = pinfo->_distance;
    else if (_quantity == PA_TANG_DISTANCE)
      retVal = pinfo->_tangential_distance;
    else if (_quantity == PA_NORMAL_X)
      retVal = pinfo->_normal(0);
    else if (_quantity == PA_NORMAL_Y)
      retVal = pinfo->_normal(1);
    else if (_quantity == PA_NORMAL_Z)
      retVal = pinfo->_normal(2);
    else if (_quantity == PA_CLOSEST_POINT_X)
      retVal = pinfo->_closest_point(0);
    else if (_quantity == PA_CLOSEST_POINT_Y)
      retVal = pinfo->_closest_point(1);
    else if (_quantity == PA_CLOSEST_POINT_Z)
      retVal = pinfo->_closest_point(2);
    else if (_quantity == PA_ELEM_ID)
      retVal = (Real) (pinfo->_elem->id()+1);
    else if (_quantity == PA_SIDE)
      retVal = (Real) (pinfo->_side_num);
    else if (_quantity == PA_INCREMENTAL_SLIP_MAG)
    {
      if (pinfo->_mech_status == PenetrationLocator::MS_NO_CONTACT)
        retVal = 0.0;
      else
        retVal = pinfo->_incremental_slip.size();
    }
    else if (_quantity == PA_INCREMENTAL_SLIP_X)
    {
      if (pinfo->_mech_status == PenetrationLocator::MS_NO_CONTACT)
        retVal = 0.0;
      else
        retVal = pinfo->_incremental_slip(0);
    }
    else if (_quantity == PA_INCREMENTAL_SLIP_Y)
    {
      if (pinfo->_mech_status == PenetrationLocator::MS_NO_CONTACT)
        retVal = 0.0;
      else
        retVal = pinfo->_incremental_slip(1);
    }
    else if (_quantity == PA_INCREMENTAL_SLIP_Z)
    {
      if (pinfo->_mech_status == PenetrationLocator::MS_NO_CONTACT)
        retVal = 0.0;
      else
        retVal = pinfo->_incremental_slip(2);
    }
    else if (_quantity == PA_ACCUMULATED_SLIP)
      retVal = pinfo->_accumulated_slip;
    else if (_quantity == PA_FORCE_X)
      retVal = pinfo->_contact_force(0);
    else if (_quantity == PA_FORCE_Y)
      retVal = pinfo->_contact_force(1);
    else if (_quantity == PA_FORCE_Z)
      retVal = pinfo->_contact_force(2);
    else if (_quantity == PA_NORMAL_FORCE_MAG)
      retVal = -pinfo->_contact_force*pinfo->_normal;
    else if (_quantity == PA_NORMAL_FORCE_X)
      retVal = (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal(0);
    else if (_quantity == PA_NORMAL_FORCE_Y)
      retVal = (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal(1);
    else if (_quantity == PA_NORMAL_FORCE_Z)
      retVal = (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal(2);
    else if (_quantity == PA_TANGENTIAL_FORCE_MAG)
    {
      RealVectorValue contact_force_normal( (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal );
      RealVectorValue contact_force_tangential( pinfo->_contact_force - contact_force_normal );
      retVal = contact_force_tangential.size();
    }
    else if (_quantity == PA_TANGENTIAL_FORCE_X)
      retVal = pinfo->_contact_force(0) - (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal(0);
    else if (_quantity == PA_TANGENTIAL_FORCE_Y)
      retVal = pinfo->_contact_force(1) - (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal(1);
    else if (_quantity == PA_TANGENTIAL_FORCE_Z)
      retVal = pinfo->_contact_force(2) - (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal(2);
    else if (_quantity == PA_FRICTIONAL_ENERGY)
      retVal = pinfo->_frictional_energy;
    else if (_quantity == PA_MECH_STATUS)
      retVal = pinfo->_mech_status;
  }

  return retVal;
}
