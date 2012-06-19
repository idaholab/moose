#include "UserObjectPPS.h"

template <>
InputParameters validParams<UserObjectPPS>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addParam<std::string>("user_data", "Name of the user data object");
  return params;
}

UserObjectPPS::UserObjectPPS(const std::string & name, InputParameters params) :
    GeneralPostprocessor(name, params),
    _mt_ud(getUserObject<MTUserObject>("user_data"))   // get user-data object and cast it down so we can use it
{
}

UserObjectPPS::~UserObjectPPS()
{
}

void
UserObjectPPS::initialize()
{
}

void
UserObjectPPS::execute()
{
}

PostprocessorValue
UserObjectPPS::getValue()
{
  return _mt_ud.getScalar();
}
