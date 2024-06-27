#pragma once
#include "closed_coil.h"
#include "div_free_source.h"
#include "named_fields_map.h"
#include "open_coil.h"
#include "scalar_potential_source.h"

namespace platypus
{

class Sources : public platypus::NamedFieldsMap<platypus::Source>
{
public:
  void Init(platypus::GridFunctions & gridfunctions,
            const platypus::FESpaces & fespaces,
            platypus::BCMap & bc_map,
            Coefficients & coefficients);
  void Apply(mfem::ParLinearForm * lf);
  void SubtractSources(mfem::ParGridFunction * gf);
};

} // namespace platypus
