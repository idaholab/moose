#include "MooseObject.h"

MooseObject::MooseObject(const std::string & name, InputParameters parameters) :
    _name(name),
    _pars(parameters)
{
}
