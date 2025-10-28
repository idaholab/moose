//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGLattice.h"

namespace CSG
{

/**
 * CSGHexagonalLattice is the class for constructing hexagonal lattices of CSGUniverses
 * arranged in concentric hexagonal rings
 */
class CSGHexagonalLattice : public CSGLattice
{
public:
  /**
   * Construct a new CSGHexagonalLattice from a map of universes.
   *
   * @param name unique identifying name of lattice
   * @param pitch flat-to-flat distance for one hexagonal lattice element
   * @param orientation orientation of the hex lattice, either "x" or "y"
   * @param universes row-wise ragged vector of universes representing hex pattern
   */
  CSGHexagonalLattice(
      const std::string & name,
      Real pitch,
      std::string orientation,
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes);

  /**
   * Construct a new empty CSGHexagonalLattice with given number of rings and pitch.
   * NOTE: must call setLatticeUniverses or addUniverseToLattice to populate.
   *
   * @param name unique identifying name of lattice
   * @param num_rings number of concentric rings around the center (>=1)
   * @param pitch flat-to-flat distance for one hexagonal lattice element
   * @param orientation orientation of the hex lattice, either "x" or "y"
   */
  CSGHexagonalLattice(const std::string & name, int num_rings, Real pitch, std::string orientation);

  /**
   * Destructor
   */
  virtual ~CSGHexagonalLattice() = default;

  /**
   * @brief Get the map of data that defines the geometric dimensions of the lattice:
   *  - num_rings: number of concentric rings in the hex lattice (int)
   *  - pitch: pitch of the lattice element (Real)
   *  - orientation: orientation of the hex lattice, either "x" or "y" (string)
   *
   * @return std::unordered_map<std::string, std::any>
   */
  virtual std::unordered_map<std::string, std::any> getDimensions() const override;

  /**
   * @brief update the specified lattice geometric dimension to the specified value.
   * Valid dimensions are: pitch (Real), num_rings (int), and orientation (string). If the universe
   * map has already been set on the lattice, num_rings and orientation cannot be updated.
   *
   * @param dim_name
   * @param dim_value
   */
  virtual void updateDimension(const std::string & dim_name, std::any dim_value) override;

  /**
   * @brief
   *
   * @param index
   * @return true
   * @return false
   */
  virtual bool isValidIndex(const std::pair<int, int> index) const override;

  /**
   * @brief
   *
   * @param universes
   * @return true
   * @return false
   */
  virtual bool isValidUniverseMap(
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes) const override;

protected:
  virtual bool hasValidDimensions() const override;
  virtual bool compareDimensions(const CSGLattice & other) const override;

private:
  int _num_rings;
  Real _pitch;
  std::string _orientation;
};

} // namespace CSG