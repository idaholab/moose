#pragma once
#include "mfem.hpp"
#include "named_fields_map.hpp"

namespace hephaestus
{

class FECollections : public hephaestus::NamedFieldsMap<mfem::FiniteElementCollection>
{
};

class FESpaces : public hephaestus::NamedFieldsMap<mfem::ParFiniteElementSpace>
{
};

class GridFunctions : public hephaestus::NamedFieldsMap<mfem::ParGridFunction>
{
};

} // namespace hephaestus
