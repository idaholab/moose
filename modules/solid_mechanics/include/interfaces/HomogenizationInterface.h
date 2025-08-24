//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "Function.h"

// Helpers common to the whole homogenization system
namespace Homogenization
{
// Moose constraint type, for input
const MultiMooseEnum constraintType("strain stress none");

/// Constraint type: stress/PK stress or strain/deformation gradient
enum class ConstraintType
{
  Strain,
  Stress,
  None
};

using ConstraintMap =
    std::map<std::pair<unsigned int, unsigned int>, std::pair<ConstraintType, const Function *>>;
}

/**
 * Interface for objects that use the homogenization constraint
 */
template <class T>
class HomogenizationInterface : public T
{
public:
  static InputParameters validParams();

  HomogenizationInterface(const InputParameters & parameters);

protected:
  /// Get the constraint map
  const Homogenization::ConstraintMap & cmap() const { return _cmap; }

private:
  /// Constraint map
  Homogenization::ConstraintMap _cmap;
};

template <class T>
InputParameters
HomogenizationInterface<T>::validParams()
{
  InputParameters params = T::validParams();
  params.addRequiredParam<MultiMooseEnum>(
      "constraint_types",
      Homogenization::constraintType,
      "Type of each constraint: strain, stress, or none. The types are specified in the "
      "column-major order, and there must be 9 entries in total.");
  params.addRequiredParam<std::vector<FunctionName>>(
      "targets", "Functions giving the targets to hit for constraint types that are not none.");
  return params;
}

template <class T>
HomogenizationInterface<T>::HomogenizationInterface(const InputParameters & parameters)
  : T(parameters)
{
  // Constraint types
  auto types = parameters.get<MultiMooseEnum>("constraint_types");
  if (types.size() != Moose::dim * Moose::dim)
    this->paramError("constraint_types",
                     "Number of constraint types must equal ",
                     Moose::dim * Moose::dim,
                     ", but ",
                     types.size(),
                     " are provided.");

  // Targets to hit
  const std::vector<FunctionName> & fnames = parameters.get<std::vector<FunctionName>>("targets");

  // Prepare the constraint map
  unsigned int fcount = 0;
  for (const auto j : make_range(Moose::dim))
    for (const auto i : make_range(Moose::dim))
    {
      const auto idx = i + Moose::dim * j;
      const auto ctype = static_cast<Homogenization::ConstraintType>(types.get(idx));
      if (ctype != Homogenization::ConstraintType::None)
      {
        if (fcount >= fnames.size())
          this->paramError(
              "targets",
              "Number of target functions must equal the number of non-none constraint "
              "types. Only ",
              fnames.size(),
              " are provided.");
        const Function * const f = &this->getFunctionByName(fnames[fcount++]);
        _cmap[{i, j}] = {ctype, f};
      }
    }

  // Make sure there aren't unused targets
  if (fcount != fnames.size())
    this->paramError(
        "targets",
        "Number of target functions must equal the number of non-none constraint types.",
        fnames.size(),
        " are provided, but ",
        fcount,
        " are used.");
}
