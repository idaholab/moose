#pragma once
#include "mfem.hpp"
#include "named_fields_map.h"

namespace platypus
{

class FECollections : public platypus::NamedFieldsMap<mfem::FiniteElementCollection>
{
};

class FESpaces : public platypus::NamedFieldsMap<mfem::ParFiniteElementSpace>
{
};

class GridFunctions : public platypus::NamedFieldsMap<mfem::ParGridFunction>
{
};

} // namespace platypus
