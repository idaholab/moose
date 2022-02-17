//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EBSDReaderPointDataAux.h"

registerMooseObject("PhaseFieldApp", EBSDReaderPointDataAux);

InputParameters
EBSDReaderPointDataAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<UserObjectName>("ebsd_reader", "The EBSDReader GeneralUserObject");
  MooseEnum field_types = EBSDAccessFunctors::getPointDataFieldType();
  params.addRequiredParam<MooseEnum>(
      "data_name", field_types, "The data to be extracted from the EBSD data by this AuxKernel");
  return params;
}

EBSDReaderPointDataAux::EBSDReaderPointDataAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _data_name(getParam<MooseEnum>("data_name")),
    _val(_ebsd_reader.getPointDataAccessFunctor(_data_name))
{
  if (isNodal())
    mooseError("This AuxKernel only supports Elemental fields");
}

void
EBSDReaderPointDataAux::precalculateValue()
{
  // EBSD data is defined at element centroids, so this only makes
  // sense as an Element AuxKernel
  Point p = _current_elem->vertex_average();

  _value = (*_val)(_ebsd_reader.getData(p));
}

Real
EBSDReaderPointDataAux::computeValue()
{
  return _value;
}
