#pragma once

#include <vector>
#include "MooseMesh.h"

/// Enum for describing the center, edge and corner subchannels or gap types
enum class EChannelType
{
  CENTER,
  EDGE,
  CORNER
};

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
   * Get the mesh nodes
   */
  virtual const std::vector<std::vector<Node *>> & getNodes() const = 0;

  /**
   * Return the number of channels per layer
   */
  virtual const unsigned int & getNumOfChannels() const = 0;

  /**
   * Return the number of gaps per layer
   */
  virtual const unsigned int & getNumOfGapsPerLayer() const = 0;

  /**
   * Return a vector of pair of subchannel indices for all gaps at a subchannel level.
   * The index pair stores the subchannel indices of the channels associated with a gap
   */
  virtual const std::vector<std::pair<unsigned int, unsigned int>> & getGapToChannelMap() const = 0;

  /**
   * Return a vector of gap indices associated with a channel
   * For each channel index we get a vector of gap indices
   */
  virtual const std::vector<std::vector<unsigned int>> & getChannelToGapMap() const = 0;

  /**
   * Return a map with gap sizes
   */
  virtual const std::vector<double> & getGapMap() const = 0;

  /**
   * Return the pitch between 2 subchannels
   */
  virtual const Real & getPitch() const = 0;

  /**
   * Return a map of signs for the cross flow
   */
  virtual const std::vector<std::vector<double>> & getSignCrossflowMap() const = 0;

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

public:
  static InputParameters validParams();
};
