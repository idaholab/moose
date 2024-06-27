#pragma once
#include "../common/pfem_extras.h"
#include "utils.h"
#include "coefficients.h"
#include "hephaestus_solvers.h"
#include "inputs.h"
#include "kernels.h"

namespace hephaestus
{

class Source : public hephaestus::Kernel<mfem::ParLinearForm>
{
public:
  Source() = default;

  // NB: must be virtual to avoid leaks (ensure correct subclass destructor!)
  ~Source() override = default;

  void Init(hephaestus::GridFunctions & gridfunctions,
            const hephaestus::FESpaces & fespaces,
            hephaestus::BCMap & bc_map,
            Coefficients & coefficients) override
  {
  }

  void Apply(mfem::ParLinearForm * lf) override = 0;
  virtual void SubtractSource(mfem::ParGridFunction * gf) = 0;
};

} // namespace hephaestus
