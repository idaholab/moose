#pragma once
#include "../common/pfem_extras.hpp"
#include "coefficients.h"
#include "hephaestus_solvers.h"
#include "inputs.h"

namespace platypus
{

template <typename T>
class Kernel
{
public:
  Kernel() = default;
  virtual ~Kernel() = default;

  Kernel(const platypus::InputParameters & params) {}
  virtual void Init(platypus::GridFunctions & gridfunctions,
                    const platypus::FESpaces & fespaces,
                    platypus::BCMap & bc_map,
                    platypus::Coefficients & coefficients)
  {
  }

  virtual void Apply(T * form) = 0;
};

} // namespace platypus
