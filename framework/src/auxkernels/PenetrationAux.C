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

#include "string_to_enum.h"

template<>
InputParameters validParams<PenetrationAux>()
{
  MooseEnum orders("FIRST, SECOND, THIRD, FORTH", "FIRST");

  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<BoundaryName>("paired_boundary", "The boundary to be penetrated");
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<std::string>("quantity","distance","The quantity to recover from the available penetration information: distance(default), tangential_distance, normal_x, normal_y, normal_z, closest_point_x, closest_point_y, closest_point_z, element_id");
  return params;
}

PenetrationAux::PenetrationAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _quantity_string( getParam<std::string>("quantity") ),
    _quantity(PA_DISTANCE),
    _penetration_locator(getPenetrationLocator(parameters.get<BoundaryName>("paired_boundary"), getParam<std::vector<BoundaryName> >("boundary")[0], Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order"))))
{
  if (parameters.isParamValid("tangential_tolerance"))
  {
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));
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
  else
    mooseError("Invalid quantity type in PenetrationAux: "<<_quantity_string);
}

Real
PenetrationAux::computeValue()
{
  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

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
  }

  return retVal;
}
