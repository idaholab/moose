//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include <memory>
#include <vector>

namespace libMesh
{
class Elem;
}

/**
 * Helper for building side pointers that avoids excessive memory allocation.
 *
 * The first time a side is requested for a certain element type, an element is built.
 * After the first call, the points are simply changed on the previously created element.
 *
 * Is thread safe when using the tid parameter.
 */
class SidePtrHelper
{
public:
  SidePtrHelper();

  /**
   * Get an element's side pointer without excessive memory allocation
   *
   * @param elem The element to build a side for
   * @param s The side to build
   * @param tid The thread id
   * @return A pointer to the side element
   */
  const libMesh::Elem *
  sidePtrHelper(const libMesh::Elem * elem, const unsigned int s, const THREAD_ID tid = 0);

private:
  /// Elements (one for each type) that are used to build element sides without extraneous allocation
  std::vector<std::vector<std::unique_ptr<const libMesh::Elem>>> _threaded_side_ptr_elems;
};
