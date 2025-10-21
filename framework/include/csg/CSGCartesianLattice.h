//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSLattice.h"

namespace CSG
{

/**
 * CSGCartesianLattice is the class for constructing regular Cartesian lattices of CSGUniverses.
 */
class CSGCartesianLattice : public CSGLattice
{
public:
  /**
   * @brief Construct a new empty Cartesian Lattice
   *
   * @param name unique name of lattice
   * @param lattice_type type of lattice
   */
  CSGCartesianLattice(const std::string & name);

  CSGCartesianLattice(
      const std::string & name,
      const Real pitch,
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes);

  CSGCartesianLattice(
    const std::string & name, const int nx0, const int nx1, const Real pitch
  )

  /**
   * Destructor
   */
  virtual ~CSGLattice() = default;

  /**
   * @brief Checks if the given index location is a valid index for the lattice
   *
   * @param index location
   * @return true if index is valid for the lattice
   */
  virtual bool isValidIndex(const std::pair<int, int> index) const override;

  /**
   * @brief given a point, check if it is within the bounds of the specified lattice
   *
   * @param point to check
   * @return true if within lattice bounds
   */
  virtual bool isPointInLattice(Point point) const override;

  /// Operator overload for checking if two CSGLattice objects are equal
  // bool operator==(const CSGLattice & other) const;

  /// Operator overload for checking if two CSGLattice objects are not equal
  // bool operator!=(const CSGLattice & other) const;

protected:
  /// given a point, get the index of the lattice element that contains it (assumes point is within bounds)
  virtual std::pair<int, int> getIndex(Point point) const override;

  /**
   * @brief For the lattice type, check that the dimension of _universe_map are valid
   *
   * @return true if valid, otherwise false
   */
  virtual bool hasValidDimensions() const override;

  /// number of elements in the first dimension
  const int _nx0;

  /// number of elements in the second direction
  const int _nx1;

  /// pitch
  const Real _pitch;
};
}
