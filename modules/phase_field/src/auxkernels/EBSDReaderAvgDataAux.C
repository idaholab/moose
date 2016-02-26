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
  const EBSDPointData & d = _ebsd_reader.getData(p);

  return (*_val)(_ebsd_reader.getAvgData(d._global));
}
