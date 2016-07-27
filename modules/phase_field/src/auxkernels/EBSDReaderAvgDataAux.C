/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "EBSDReaderAvgDataAux.h"

template<>
InputParameters validParams<EBSDReaderAvgDataAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("ebsd_reader", "The EBSDReader GeneralUserObject");
  MooseEnum field_types = EBSDAccessFunctors::getAvgDataFieldType();
  params.addRequiredParam<MooseEnum>("data_name", field_types, "The averaged data to be extracted from the EBSD data by this AuxKernel");
  return params;
}

EBSDReaderAvgDataAux::EBSDReaderAvgDataAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _data_name(getParam<MooseEnum>("data_name")),
    _val(_ebsd_reader.getAvgDataAccessFunctor(_data_name))
{
}

Real
EBSDReaderAvgDataAux::computeValue()
{
  // EBSD data is defined at element centroids, so this only makes
  // sense as an Element AuxKernel
  Point p = _current_elem->centroid();

  // get the EBSD Point data for the current element
  const EBSDPointData & d = _ebsd_reader.getData(p);

  // get the (global) grain ID for the EBSD feature ID
  const unsigned int global_id = _ebsd_reader.getGlobalID(d._feature_id);

  // look up average data for the given (global) grain ID
  return (*_val)(_ebsd_reader.getAvgData(global_id));
}
