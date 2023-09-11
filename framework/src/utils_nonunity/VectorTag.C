//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorTag.h"

VectorTag::VectorTag()
  : _id(Moose::INVALID_TAG_ID),
    _type_id(Moose::INVALID_TAG_TYPE_ID),
    _name(""),
    _type(Moose::VECTOR_TAG_ANY)
{
}

VectorTag::VectorTag(const TagID id,
                     const TagTypeID type_id,
                     const TagName name,
                     const Moose::VectorTagType type)
  : _id(id), _type_id(type_id), _name(name), _type(type)
{
}

bool
VectorTag::operator==(const VectorTag & other) const
{
  return _id == other._id && _type_id == other._type_id && _name == other._name &&
         _type == other._type;
}
