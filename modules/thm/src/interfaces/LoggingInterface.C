#include "LoggingInterface.h"

LoggingInterface::LoggingInterface(THMApp & app, const std::string & name)
  : _logging_app(app), _logging_obj_name(name), _logging_obj_has_name(_logging_obj_name != "")
{
}
