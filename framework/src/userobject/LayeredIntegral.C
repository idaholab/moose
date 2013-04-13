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

#include "LayeredIntegral.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

template<>
InputParameters validParams<LayeredIntegral>()
{
  InputParameters params = validParams<ElementIntegralVariableUserObject>();
  params += validParams<LayeredBase>();

  params.set<std::string>("built_by_action") = "add_user_object";

  return params;
}

LayeredIntegral::LayeredIntegral(const std::string & name, InputParameters parameters) :
    ElementIntegralVariableUserObject(name, parameters),
    LayeredBase(name, parameters)
{}

void
LayeredIntegral::initialize()
{
  ElementIntegralVariableUserObject::initialize();
  LayeredBase::initialize();
}

void
LayeredIntegral::execute()
{
  Real integral_value = computeIntegral();

  unsigned int layer = getLayer(_current_elem->centroid());

  setLayerValue(layer, getLayerValue(layer) + integral_value);
}

void
LayeredIntegral::finalize()
{
  LayeredBase::finalize();
}

void
LayeredIntegral::threadJoin(const UserObject & y)
{
  ElementIntegralVariableUserObject::threadJoin(y);
  LayeredBase::threadJoin(y);
}
