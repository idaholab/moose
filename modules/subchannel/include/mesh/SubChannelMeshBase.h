#pragma once

#include <vector>
#include "MooseMesh.h"
#include "SubChannelEnums.h"

/**
 * Base class for subchannel meshes
 */
class SubChannelMeshBase : public MooseMesh
{
public:
  SubChannelMeshBase(const InputParameters & parameters);
  SubChannelMeshBase(const SubChannelMeshBase & other_mesh);

  /**
   * Get axial location of layers
   */
  virtual const std::vector<Real> & getZGrid() const { return _z_grid; }

  /**
   * Return the number of axial nodes
   */
  virtual const unsigned int & getNumOfAxialNodes() const { return _nz; }

  /**
   * Get the mesh node for a given channel index and elevation index
   */
  virtual Node * getChannelNode(unsigned int i_chan, unsigned iz) const = 0;

  /**
   * Return the number of channels per layer
   */
  virtual const unsigned int & getNumOfChannels() const = 0;

  /**
   * Return the number of gaps per layer
   */
  virtual const unsigned int & getNumOfGapsPerLayer() const = 0;

  /**
   * Return a pair of subchannel indices for a given gap index
   */
  virtual const std::pair<unsigned int, unsigned int> &
  getGapNeighborChannels(unsigned int i_gap) const = 0;

  /**
   * Return a vector of gap indices for a given channel index
   */
  virtual const std::vector<unsigned int> & getChannelGaps(unsigned int i_chan) const = 0;

  /**
   * Return a map with gap sizes
   */
  virtual const std::vector<double> & getGapMap() const = 0;

  /**
   * Return the pitch between 2 subchannels
   */
  virtual const Real & getPitch() const { return _pitch; }

  /**
   * Return rod diameter
   */
  virtual const Real & getRodDiameter() const { return _rod_diameter; }

  /**
   * Return a signs for the cross flow given a subchannel index and local neighbor index
   */
  virtual const Real & getCrossflowSign(unsigned int i_chan, unsigned int i_local) const = 0;

  /**
   * Return heated length
   */
  virtual const Real & getHeatedLength() const { return _heated_length; }

  /**
   * Return a subchannel index for a given physical point `p`
   */
  virtual unsigned int getSubchannelIndexFromPoint(const Point & p) const = 0;

  /**
   * Return the type of the subchannel for given subchannel index
   */
  virtual EChannelType getSubchannelType(unsigned int index) const = 0;

  /**
   * Return gap width for a given gap index
   */
  virtual Real getGapWidth(unsigned int gap_index) const = 0;

protected:
  /// number of axial nodes
  unsigned int _nz;
  /// heated length of the fuel rod
  Real _heated_length;
  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// axial location of the spacers
  const std::vector<Real> & _spacer_z;
  /// form loss coefficient of the spacers
  const std::vector<Real> & _spacer_k;
  /// max allowed axial node size
  Real _max_dz;
  /// Distance between the neighbor fuel rods, pitch
  Real _pitch;
  /// fuel rod diameter
  Real _rod_diameter;

public:
  static InputParameters validParams();
};
