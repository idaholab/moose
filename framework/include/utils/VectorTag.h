//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseTypes.h"

/**
 * Storage for all of the information pretaining to a vector tag
 */
class VectorTag
{
public:
  VectorTag();

  VectorTag(const TagID id, const TagName name, const Moose::VectorTagType type);

  bool operator==(const VectorTag & other) const;

  /// The id associated with the vector tag
  TagID _id;
  /// The name of the vector tag
  TagName _name;
  /// The type of the vector tag
  Moose::VectorTagType _type;
};
