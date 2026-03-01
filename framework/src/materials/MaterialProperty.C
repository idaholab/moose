//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialProperty.h"

template <typename Context>
void
dataStore(std::ostream & stream, PropertyValue & p, Context)
{
  p.store(stream);
}

template void dataStore(std::ostream & stream, PropertyValue & p, void * context);

template <typename Context>
void
dataLoad(std::istream & stream, PropertyValue & p, Context)
{
  p.load(stream);
}

template void dataLoad(std::istream & stream, PropertyValue & p, void * context);

template <typename Context>
void
dataStore(std::ostream & stream, MaterialProperties & v, Context context)
{
  std::size_t prop_size = v.size();
  dataStore(stream, prop_size, context);

  for (const auto i : index_range(v))
    dataStore(stream, v[i], context);
}

template void dataStore(std::ostream & stream, MaterialProperties & v, void * context);

template <typename Context>
void
dataLoad(std::istream & stream, MaterialProperties & v, Context context)
{
  std::size_t prop_size;
  dataLoad(stream, prop_size, context);
  mooseAssert(prop_size == v.size(), "Loading MaterialProperties data into mis-sized target");

  for (const auto i : make_range(prop_size))
    dataLoad(stream, v[i], context);
}

template void dataLoad(std::istream & stream, MaterialProperties & v, void * context);
