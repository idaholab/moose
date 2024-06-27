#pragma once
#include "closed_coil.h"
#include "div_free_source.h"
#include "named_fields_map.h"
#include "open_coil.h"
#include "scalar_potential_source.h"

namespace hephaestus
{

class Sources : public hephaestus::NamedFieldsMap<hephaestus::Source>
{
public:
  void Init(hephaestus::GridFunctions & gridfunctions,
            const hephaestus::FESpaces & fespaces,
            hephaestus::BCMap & bc_map,
            Coefficients & coefficients);
  void Apply(mfem::ParLinearForm * lf);
  void SubtractSources(mfem::ParGridFunction * gf);
};

} // namespace hephaestus
