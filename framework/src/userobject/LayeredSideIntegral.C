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

#include "LayeredSideIntegral.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

template<>
InputParameters validParams<LayeredSideIntegral>()
{
  InputParameters params = validParams<SideIntegralVariableUserObject>();
  params += validParams<LayeredBase>();

  params.set<std::string>("built_by_action") = "add_user_object";

  return params;
}

LayeredSideIntegral::LayeredSideIntegral(const std::string & name, InputParameters parameters) :
    SideIntegralVariableUserObject(name, parameters),
    LayeredBase(name, parameters)
{}

void
LayeredSideIntegral::initialize()
{
  SideIntegralVariableUserObject::initialize();
  LayeredBase::initialize();
}

void
LayeredSideIntegral::execute()
{
  Real integral_value = computeIntegral();

  unsigned int layer = getLayer(_current_elem->centroid());

  setLayerValue(layer, getLayerValue(layer) + integral_value);
}

void
LayeredSideIntegral::finalize()
{
  LayeredBase::finalize();
}

void
LayeredSideIntegral::threadJoin(const UserObject & y)
{
  SideIntegralVariableUserObject::threadJoin(y);
  LayeredBase::threadJoin(y);
}
