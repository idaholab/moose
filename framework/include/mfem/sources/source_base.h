#pragma once
#include "../common/pfem_extras.hpp"
#include "utils.h"
#include "coefficients.h"
#include "hephaestus_solvers.h"
#include "inputs.h"
#include "kernels.h"

namespace platypus
{

class Source : public platypus::Kernel<mfem::ParLinearForm>
{
public:
  Source() = default;

  // NB: must be virtual to avoid leaks (ensure correct subclass destructor!)
  ~Source() override = default;

  void Init(platypus::GridFunctions & gridfunctions,
            const platypus::FESpaces & fespaces,
            platypus::BCMap & bc_map,
            platypus::Coefficients & coefficients) override
  {
  }

  void Apply(mfem::ParLinearForm * lf) override = 0;
  virtual void SubtractSource(mfem::ParGridFunction * gf) = 0;
};

} // namespace platypus
