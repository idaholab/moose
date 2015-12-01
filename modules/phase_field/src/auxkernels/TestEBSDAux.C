/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TestEBSDAux.h"

template<>
InputParameters validParams<TestEBSDAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("ebsd_reader", "The EBSDReader GeneralUserObject");
  MooseEnum field_types = EBSDAccessFunctors::getPointDataFieldType();
  params.addRequiredParam<MooseEnum>("data_name", field_types, "The data to be extracted from the EBSD data by this AuxKernel");
  return params;
}

TestEBSDAux::TestEBSDAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _data_name(getParam<MooseEnum>("data_name")),
    _val(_ebsd_reader.getPointDataAccessFunctor(_data_name))
{
}

TestEBSDAux::~TestEBSDAux()
{
}

Real
TestEBSDAux::computeValue()
{
  // EBSD data is defined at element centroids, so this only makes
  // sense as an Element AuxKernel
  Point p = _current_elem->centroid();

  return (*_val)(_ebsd_reader.getData(p));
}

