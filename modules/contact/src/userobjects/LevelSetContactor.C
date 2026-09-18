//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetContactor.h"
#include "Function.h"

namespace
{
constexpr std::array<const char *, 3> AXIS_NAMES = {"x", "y", "z"};
}

InputParameters
LevelSetContactor::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Base class for a rigid contactor described implicitly by a "
                             "signed-distance (level-set) function, optionally translated in "
                             "each Cartesian axis by a Function or a coupled Scalar variable.");
  for (const auto axis : AXIS_NAMES)
  {
    params.addParam<FunctionName>(std::string("disp_") + axis + "_function",
                                  "Optional Function giving the contactor's rigid translation "
                                  "along the " +
                                      std::string(axis) +
                                      " axis.  Mutually exclusive with "
                                      "`disp_" +
                                      std::string(axis) + "_scalar`.");
    params.addCoupledVar(std::string("disp_") + axis + "_scalar",
                         "Optional coupled Scalar variable giving the contactor's rigid "
                         "translation along the " +
                             std::string(axis) + " axis.  Mutually exclusive with `disp_" +
                             std::string(axis) + "_function`.");
  }
  return params;
}

LevelSetContactor::LevelSetContactor(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _function{{nullptr, nullptr, nullptr}},
    _scalar{{nullptr, nullptr, nullptr}}
{
  _scalar_var_num.fill(libMesh::invalid_uint);
  for (const auto k : {0u, 1u, 2u})
  {
    const std::string fname = std::string("disp_") + AXIS_NAMES[k] + "_function";
    const std::string sname = std::string("disp_") + AXIS_NAMES[k] + "_scalar";
    const bool has_func = isParamValid(fname);
    const bool has_scal = isCoupledScalar(sname);
    if (has_func && has_scal)
      paramError(fname,
                 "`",
                 fname,
                 "` and `",
                 sname,
                 "` are mutually exclusive: pick one input per axis.");
    if (has_func)
      _function[k] = &getFunction(fname);
    else if (has_scal)
    {
      _scalar[k] = &coupledScalarValue(sname);
      _scalar_var_num[k] = coupledScalar(sname);
    }
  }
}

Point
LevelSetContactor::translation() const
{
  Point t;
  for (const auto k : {0u, 1u, 2u})
  {
    if (_scalar[k] != nullptr)
      t(k) = (*_scalar[k])[0];
    else if (_function[k] != nullptr)
      t(k) = _function[k]->value(_t, Point());
  }
  return t;
}
