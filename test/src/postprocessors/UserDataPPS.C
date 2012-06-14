#include "UserDataPPS.h"

template <>
InputParameters validParams<UserDataPPS>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addParam<std::string>("user_data", "Name of the user data object");
  return params;
}

UserDataPPS::UserDataPPS(const std::string & name, InputParameters params) :
    GeneralPostprocessor(name, params),
    _mt_ud(dynamic_cast<const MTUserData &>(getUserObject("user_data")))   // get user-data object and cast it down so we can use it
{
}

UserDataPPS::~UserDataPPS()
{
}

void
UserDataPPS::initialize()
{
}

void
UserDataPPS::execute()
{
}

PostprocessorValue
UserDataPPS::getValue()
{
  return _mt_ud.getScalar();
}
