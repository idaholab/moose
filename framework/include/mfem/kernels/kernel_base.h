#pragma once
#include "../common/pfem_extras.hpp"
#include "coefficients.h"
#include "hephaestus_solvers.h"
#include "inputs.h"

namespace hephaestus
{

template <typename T>
class Kernel
{
public:
  Kernel() = default;
  virtual ~Kernel() = default;

  Kernel(const hephaestus::InputParameters & params) {}
  virtual void Init(hephaestus::GridFunctions & gridfunctions,
                    const hephaestus::FESpaces & fespaces,
                    hephaestus::BCMap & bc_map,
                    Coefficients & coefficients)
  {
  }

  virtual void Apply(T * form) = 0;
};

} // namespace hephaestus
