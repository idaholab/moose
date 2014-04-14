#include "TestEBSDAux.h"

template<>
InputParameters validParams<TestEBSDAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("ebsd_reader", "The EBSDReader GeneralUserObject");
  params.addRequiredParam<std::string>("data_name", "The data to be extracted from the EBSD data by this AuxKernel");

  return params;
}

TestEBSDAux::TestEBSDAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _data_name(getParam<std::string>("data_name")),
    _data_type(_ebsd_reader.getDataType(_data_name))
{
}


Real
TestEBSDAux::computeValue()
{
  // EBSD data is defined at element centroids, so this only makes
  // sense as an Element AuxKernel
  Point p = _current_elem->centroid();

  Real ret_val = _ebsd_reader.get_data(p, _data_type);

  // Debugging
  // Moose::out << "Returning value " << ret_val << std::endl;
  return ret_val;
}
