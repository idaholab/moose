/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PolycrystalEBSD.h"
#include "EBSDReader.h"

template <>
InputParameters
validParams<PolycrystalEBSD>()
{
  InputParameters params = validParams<PolycrystalUserObjectBase>();
  params.addClassDescription("Object for setting up a polycrystal structure from an EBSD Datafile");
  params.addParam<UserObjectName>("ebsd_reader", "EBSD Reader for initial condition");
  params.addParam<unsigned int>("phase", "EBSD phase number from which to retrieve information");
  return params;
}

PolycrystalEBSD::PolycrystalEBSD(const InputParameters & parameters)
  : PolycrystalUserObjectBase(parameters),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _consider_phase(isParamValid("phase")),
    _phase(isParamValid("phase") ? getParam<unsigned int>("phase") : 0)
{
}

unsigned int
PolycrystalEBSD::getGrainBasedOnPoint(const Point & point) const
{
  const EBSDAccessFunctors::EBSDPointData & d = _ebsd_reader.getData(point);
  const auto phase = d._phase;

  // See if we are in a phase that we are actually tracking
  if (_consider_phase && phase != _phase)
    return 0;

  // Get the ids from the EBSD reader
  const auto global_id = _ebsd_reader.getGlobalID(d._feature_id);
  const auto local_id = _ebsd_reader.getAvgData(global_id)._local_id;

  return _consider_phase ? local_id : global_id;
}
