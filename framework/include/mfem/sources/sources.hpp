#pragma once
#include "closed_coil.hpp"
#include "div_free_source.hpp"
#include "named_fields_map.hpp"
#include "open_coil.hpp"
#include "scalar_potential_source.hpp"

namespace hephaestus
{

class Sources : public hephaestus::NamedFieldsMap<hephaestus::Source>
{
public:
  void Init(hephaestus::GridFunctions & gridfunctions,
            const hephaestus::FESpaces & fespaces,
            hephaestus::BCMap & bc_map,
            hephaestus::Coefficients & coefficients);
  void Apply(mfem::ParLinearForm * lf);
  void SubtractSources(mfem::ParGridFunction * gf);
};

} // namespace hephaestus
