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

  VectorTag(const TagID id,
            const unsigned int type_index,
            const TagName name,
            const Moose::VectorTagType type);

  bool operator==(const VectorTag & other) const;

  /// The id associated with the vector tag
  TagID _id;

  /**
   * The index for this tag into a vector that contains tags of only its type ordered by ID
   *
   * This is specifically meant for indexing into the result of SubProblem::getVectorTags(type),
   * where type is not VECTOR_TAG_ANY
   *
   * Example:
   * | _id | _type_id |        _type        |
   * ----------------------------------------
   * |  0  |     0    | VECTOR_TAG_RESIDUAL |
   * |  1  |     1    | VECTOR_TAG_RESIDUAL |
   * |  2  |     0    | VECTOR_TAG_SOLUTION |
   * |  3  |     2    | VECTOR_TAG_RESIDUAL |
   * |  4  |     1    | VECTOR_TAG_SOLUTION |
   */
  TagTypeID _type_id;

  /// The name of the vector tag
  TagName _name;

  /// The type of the vector tag
  Moose::VectorTagType _type;
};
