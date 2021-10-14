//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "MooseEnum.h"
#include "DelimitedFileReader.h"

/**
 * Read properties from file - grain, element, or block
 * Input file syntax: prop1 prop2 etc. See test.
 * For grain level, voronoi tesellation with random grain centers are generated;
 * Element center points used for assigning properties
 * Usable for generated mesh
 * For block type, elements inside one block are assigned identical material properties;
 */

class ElementPropertyReadFile : public GeneralUserObject
{
public:
  static InputParameters validParams();

  ElementPropertyReadFile(const InputParameters & parameters);
  virtual ~ElementPropertyReadFile() {}

  virtual void initialize() {}
  virtual void execute() {}
  virtual void finalize() {}

  /**
   * This function reads data from file
   */
  void readData();

  /**
   * This function generates grain center point
   * Presently random generated
   */
  virtual void initGrainCenterPoints();

  /**
   * This function assign property data to elements
   */
  Real getData(const Elem *, unsigned int) const;

  /**
   * This function assign properties to element read from file with element based properties
   */
  Real getElementData(const Elem *, unsigned int) const;

  /**
   * This function assign properties to element read from file with grain  based properties
   * Grain distribution in the RVE can be Periodic or non-periodic (default)
   */
  Real getGrainData(const Elem *, unsigned int) const;

  /**
   * This function assigns properties to elements read from file with block  based properties
   */
  Real getBlockData(const Elem *, unsigned int) const;

  /**
   * This function calculates minimum distance between 2 points
   * considering periodicity of the simulation volume
   */
  Real minPeriodicDistance(Point, Point) const;

protected:
  ///Name of file containing property values
  std::string _prop_file_name;
  ///Use DelimitedFileReader to read and store data from file
  MooseUtils::DelimitedFileReader _reader;
  ///Number of properties in a row
  unsigned int _nprop;
  ///Number of grains (for property read based on grains)
  unsigned int _ngrain;
  ///Number of blocks (for property read based on blocks)
  unsigned int _nblock;
  ///Type of read - element, grain, or block
  const enum class ReadType { ELEMENT, GRAIN, BLOCK } _read_type;
  ///Random seed - used for generating grain centers
  unsigned int _rand_seed;
  ///Type of grain structure - non-periodic default
  MooseEnum _rve_type;
  /// Do the block numbers start with zero or one?
  bool _block_zero;

  MooseMesh & _mesh;
  std::vector<Point> _center;

private:
  unsigned int _nelem;
  Point _top_right;
  Point _bottom_left;
  Point _range;
  Real _max_range;
};
