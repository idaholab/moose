//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "JsonOutputUtils.h"

namespace JsonOutputUtils
{

nlohmann::json
anyToJson(const std::any & data)
{
  if (data.type() == typeid(int))
    return nlohmann::json(std::any_cast<int>(data));
  else if (data.type() == typeid(unsigned int))
    return nlohmann::json(std::any_cast<unsigned int>(data));
  else if (data.type() == typeid(std::string))
    return nlohmann::json(std::any_cast<std::string>(data));
  else if (data.type() == typeid(Real))
    return nlohmann::json(std::any_cast<Real>(data));
  else if (data.type() == typeid(bool))
    return nlohmann::json(std::any_cast<bool>(data));
  else if (data.type() == typeid(std::vector<int>))
    return nlohmann::json(std::any_cast<std::vector<int>>(data));
  else if (data.type() == typeid(std::vector<unsigned int>))
    return nlohmann::json(std::any_cast<std::vector<unsigned int>>(data));
  else if (data.type() == typeid(std::vector<std::string>))
    return nlohmann::json(std::any_cast<std::vector<std::string>>(data));
  else if (data.type() == typeid(std::vector<Real>))
    return nlohmann::json(std::any_cast<std::vector<Real>>(data));
  else if (data.type() == typeid(std::vector<bool>))
    return nlohmann::json(std::any_cast<std::vector<bool>>(data));
  else
    mooseError("Unsupported any data type to convert to JSON.");
}

nlohmann::json
variantToJson(const AttributeVariant & data)
{
  return std::visit([](const auto & value) -> nlohmann::json { return nlohmann::json(value); },
                    data);
}
}
