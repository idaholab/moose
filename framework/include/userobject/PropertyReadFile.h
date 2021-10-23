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
 * How data is organized in the CSV file
 */
enum class ReadTypeEnum
{
  ELEMENT = 0,
  VORONOI = 1,
  BLOCK = 2,
  NODE = 3
};

/**
 * Read properties from file - grain, element, node or block
 * Input file syntax: prop1 prop2 etc. See test.
 * For grain level, voronoi tesellation with random grain centers are generated;
 * Element center points used for assigning properties
 * Usable for generated mesh
 * For sorting by elements: the CSV data should be sorted by element ID
 * For sorting by nodes: the CSV data should be organized by node ID
 * For block type, elements inside one block are assigned identical material properties;
 */

class PropertyReadFile : public GeneralUserObject
{
public:
  static InputParameters validParams();

  PropertyReadFile(const InputParameters & parameters);
  virtual ~PropertyReadFile() {}

  virtual void initialize() {}
  virtual void execute() {}
  virtual void finalize() {}

  /**
   * This function reads the data from file
   */
  void readData();

  /**
   * This function generates voronoi tesselation center points
   * Presently random generated
   */
  virtual void initVoronoiCenterPoints();

  /**
   * This function assign property data to elements
   * @param elem the element to get data for
   * @param prop_num the column index of the property we want to retrieve
   */
  Real getData(const Elem * elem, unsigned int prop_num) const;

  /**
   * This function assign properties to element read from file with element based properties
   * @param elem the element to get data for
   * @param prop_num the column index of the property we want to retrieve
   */
  Real getElementData(const Elem * elem, unsigned int prop_num) const;

  /**
   * This function assign properties to read from file with node based properties
   * @param node the node to get the data for
   * @param prop_num the column index of the property we want to retrieve
   */
  Real getNodeData(const Node * node, unsigned int prop_num) const;

  /**
   * This function assign properties to element read from file with nearest neighbor / grain based
   * properties Voronoi centers distribution in the RVE can be Periodic or non-periodic (default)
   * @param centroid the centroid of the element to get data for
   * @param prop_num the column index of the property we want to retrieve
   */
  Real getVoronoiData(const Point centroid, unsigned int prop_num) const;

  /**
   * This function assigns properties to elements read from file with block  based properties
   * @param elem the element to get data for
   * @param prop_num the column index of the property we want to retrieve
   */
  Real getBlockData(const Elem * elem, unsigned int prop_num) const;

  /**
   * This function calculates minimum distance between 2 points
   * considering periodicity of the simulation volume
   */
  Real minPeriodicDistance(Point, Point) const;

  /**
   * Returns the ordering of data expected in the CSV file
   */
  ReadTypeEnum getReadType() const { return _read_type; }

protected:
  /// Name of file containing property values
  const std::string _prop_file_name;
  /// Use DelimitedFileReader to read and store data from file
  MooseUtils::DelimitedFileReader _reader;
  /// Number of properties in a row
  const unsigned int _nprop;
  /// Number of grains (for property read based on grains)
  const unsigned int _nvoronoi;
  /// Number of blocks (for property read based on blocks)
  const unsigned int _nblock;
  /// Type of read - element, grain, or block
  const ReadTypeEnum _read_type;
  /// Whether to use a random tesselation for the Voronoi/grain type
  const bool _use_random_tesselation;
  /// Random seed - used for generating grain centers
  const unsigned int _rand_seed;
  /// Type of voronoi tesselation/grain structure - non-periodic default
  const MooseEnum _rve_type;
  /// Do the block numbers start with zero or one?
  bool _block_zero;

  /// To pass CI TODO: delete
  const unsigned int _ngrain;

  MooseMesh & _mesh;
  std::vector<Point> _center;

private:
  unsigned int _nelem;
  unsigned int _nnodes;
  Point _top_right;
  Point _bottom_left;
  Point _range;
  Real _max_range;
};
