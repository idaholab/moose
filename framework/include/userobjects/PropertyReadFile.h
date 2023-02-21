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
   * This function retrieves property data for elements
   * @param elem the element to get data for
   * @param prop_num the column index of the property we want to retrieve
   * @return the property value for the element
   */
  Real getData(const Elem * const elem, const unsigned int prop_num) const;

  /**
   * This function retrieves properties for elements, from a file that has element-based data
   * @param elem the element to get data for
   * @param prop_num the column index of the property we want to retrieve
   * @return the property value for the element
   */
  Real getElementData(const Elem * const elem, const unsigned int prop_num) const;

  /**
   * This function retrieves properties for nodes, from a file that has node-based data
   * @param node the node to get the data for
   * @param prop_num the column index of the property we want to retrieve
   * @return the property value for the node
   */
  Real getNodeData(const Node * const node, const unsigned int prop_num) const;

  /**
   * This function retrieves properties for elements from a file with nearest neighbor / grain based
   * properties. Voronoi centers distribution in the RVE can be Periodic or non-periodic (default)
   * @param point the location to get data for
   * @param prop_num the column index of the property we want to retrieve
   * @return the property value for the element
   */
  Real getVoronoiData(const Point & point, const unsigned int prop_num) const;

  /**
   * This function retrieves properties for elements, from a file that has block-based data
   * @param elem the element to get data for
   * @param prop_num the column index of the property we want to retrieve
   * @return the property value for the element
   */
  Real getBlockData(const Elem * const elem, const unsigned int prop_num) const;

  /**
   * This function calculates minimum distance between 2 points
   * considering periodicity of the simulation volume
   * @return the minimum distance between two points
   */
  Real minPeriodicDistance(const Point &, const Point &) const;

  /**
   * How data is organized in the CSV file
   */
  enum class ReadTypeEnum
  {
    ELEMENT = 0,
    VORONOI = 1,
    BLOCK = 2,
    NODE = 3,
    GRAIN = 4
  };

  /**
   * Returns the ordering of data expected in the CSV file
   */
  ReadTypeEnum getReadType() const { return _read_type; }

  /**
   * Returns the number of properties (columns) read in the file
   */
  unsigned int getNumProperties() const { return _nprop; }

protected:
  /// Name of file containing property values
  const std::string _prop_file_name;
  /// Use DelimitedFileReader to read and store data from file
  MooseUtils::DelimitedFileReader _reader;

  /// Type of read - element, grain, or block
  const ReadTypeEnum _read_type;

  /// Parameters for the nearest neighbor / grain interpolation
  /// Whether to use a random tesselation for the Voronoi/grain type
  const bool _use_random_tesselation;
  /// Random seed - used for generating grain centers
  const unsigned int _rand_seed;
  /// Type of voronoi tesselation/grain structure - non-periodic default
  const MooseEnum _rve_type;
  /// Do the block numbers start with zero or one?
  bool _block_zero;

  /// Legacy attribute to keep Grizzly functional, see idaholab/moose#19109, idaholab/Grizzly#182
  const unsigned int _ngrain;

  MooseMesh & _mesh;
  std::vector<Point> _center;

private:
  /// Bounding box for the mesh
  BoundingBox _bounding_box;

  /// Class attributes useful for range-checking
  /// Number of properties in a row
  const unsigned int _nprop;
  /// Number of grains (for reading a CSV file with properties ordered by grains)
  const unsigned int _nvoronoi;
  /// Number of blocks (for reading a CSV file with properties ordered by blocks)
  const unsigned int _nblock;
};
