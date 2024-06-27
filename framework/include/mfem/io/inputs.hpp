#pragma once
#include <any>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>

#include "boundary_conditions.hpp"
#include "coefficients.hpp"
#include "outputs.hpp"

namespace hephaestus
{

class InputParameters
{

protected:
  std::map<std::string, std::any> _params;

public:
  InputParameters() = default;
  InputParameters(std::map<std::string, std::any> _params) : _params(std::move(_params)) {}
  void SetParam(std::string param_name, std::any value) { _params[param_name] = value; };
  template <typename T>
  [[nodiscard]] [[nodiscard]] T GetParam(std::string param_name) const
  {
    T param;
    try
    {
      param = std::any_cast<T>(_params.at(param_name));
    }
    catch (const std::exception & e)
    {
      MFEM_ABORT("Exception raised when trying to cast required parameter '" << param_name
                                                                             << "': " << e.what());
    }
    return param;
  };
  template <typename T>
  [[nodiscard]] [[nodiscard]] [[nodiscard]] T GetOptionalParam(std::string param_name,
                                                               T value) const
  {
    T param;
    try
    {
      param = std::any_cast<T>(_params.at(param_name));
    }
    catch (...)
    {
      param = value;
    }
    return param;
  };
};

} // namespace hephaestus
