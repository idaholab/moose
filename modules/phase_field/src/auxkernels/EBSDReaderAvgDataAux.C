/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "EBSDReaderAvgDataAux.h"
#include "EBSDReader.h"
#include "GrainTrackerInterface.h"

template <>
InputParameters
validParams<EBSDReaderAvgDataAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("ebsd_reader", "The EBSDReader GeneralUserObject");
  params.addRequiredParam<UserObjectName>("grain_tracker", "The GrainTracker UserObject");
  MooseEnum field_types = EBSDAccessFunctors::getAvgDataFieldType();
  params.addRequiredParam<MooseEnum>(
      "data_name",
      field_types,
      "The averaged data to be extracted from the EBSD data by this AuxKernel");
  params.addParam<Real>("invalid", -1.0, "Value to return for points without active grains.");
  return params;
}

EBSDReaderAvgDataAux::EBSDReaderAvgDataAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker")),
    _data_name(getParam<MooseEnum>("data_name")),
    _val(_ebsd_reader.getAvgDataAccessFunctor(_data_name)),
    _invalid(getParam<Real>("invalid"))
{
}

void
EBSDReaderAvgDataAux::precalculateValue()
{
  // get the dominant grain for the current element/node
  const int grain_id =
      _grain_tracker.getEntityValue(isNodal() ? _current_node->id() : _current_elem->id(),
                                    FeatureFloodCount::FieldType::UNIQUE_REGION,
                                    0);

  // no grain found
  if (grain_id < 0)
    _value = _invalid;

  // get the data for the grain
  else
    _value = (*_val)(_ebsd_reader.getAvgData(grain_id));
}

Real
EBSDReaderAvgDataAux::computeValue()
{
  return _value;
}
