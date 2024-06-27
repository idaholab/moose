#include "sources.h"

namespace platypus
{

void
Sources::Init(platypus::GridFunctions & gridfunctions,
              const platypus::FESpaces & fespaces,
              platypus::BCMap & bc_map,
              Coefficients & coefficients)
{
  for (const auto & [name, source] : *this)
  {
    source->Init(gridfunctions, fespaces, bc_map, coefficients);
  }
}

void
Sources::Apply(mfem::ParLinearForm * lf)
{
  for (const auto & [name, source] : *this)
  {
    source->Apply(lf);
  }
}

void
Sources::SubtractSources(mfem::ParGridFunction * gf)
{
  for (const auto & [name, source] : *this)
  {
    source->SubtractSource(gf);
  }
}

} // namespace platypus
