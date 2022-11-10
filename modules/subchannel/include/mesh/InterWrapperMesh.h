/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#pragma once

#include <vector>
#include "MooseMesh.h"
#include "SubChannelEnums.h"

/**
 * Base class for inter-wrapper meshes
 */
class InterWrapperMesh : public MooseMesh
{
public:
  InterWrapperMesh(const InputParameters & parameters);
  InterWrapperMesh(const InterWrapperMesh & other_mesh);

  /**
   * Get axial location of layers
   */
  virtual const std::vector<Real> & getZGrid() const { return _z_grid; }

  virtual unsigned int getZIndex(const Point & point) const;

  /**
   * Get axial cell location and value of loss coefficient
   */
  virtual const std::vector<std::vector<Real>> & getKGrid() const { return _k_grid; }

  /**
   * Return lateral loss coefficient
   */
  virtual const Real & getKij() const { return _kij; }

  /**
   * Return the number of axial cells
   */
  virtual const unsigned int & getNumOfAxialCells() const { return _n_cells; }

  /**
   * Get the inter-wrapper mesh node for a given channel index and elevation index
   */
  virtual Node * getChannelNode(unsigned int i_chan, unsigned iz) const = 0;

  /**
   * Get the pin mesh node for a given pin index and elevation index
   */
  virtual Node * getPinNode(unsigned int i_pin, unsigned iz) const = 0;

  /**
   * Return the number of channels per layer
   */
  virtual const unsigned int & getNumOfChannels() const = 0;

  /**
   * Return if Pin Mesh exists or not
   */
  virtual bool pinMeshExist() const = 0;

  /**
   * Return if Pin Mesh exists or not
   */
  virtual bool ductMeshExist() const = 0;

  /**
   * Return the number of gaps per layer
   */
  virtual const unsigned int & getNumOfGapsPerLayer() const = 0;

  /**
   * Return the number of pins
   */
  virtual const unsigned int & getNumOfPins() const = 0;

  /**
   * Return a pair of inter-wrapper indices for a given gap index
   */
  virtual const std::pair<unsigned int, unsigned int> &
  getGapNeighborChannels(unsigned int i_gap) const = 0;

  /**
   * Return a vector of gap indices for a given channel index
   */
  virtual const std::vector<unsigned int> & getChannelGaps(unsigned int i_chan) const = 0;

  /**
   * Return a vector of channel indices for a given Pin index
   */
  virtual const std::vector<unsigned int> & getPinChannels(unsigned int i_pin) const = 0;

  /**
   * Return a vector of pin indices for a given channel index
   */
  virtual const std::vector<unsigned int> & getChannelPins(unsigned int i_chan) const = 0;

  /**
   * Return a map with gap sizes
   */
  virtual const std::vector<double> & getGapMap() const = 0;

  /**
   * Return the pitch between 2 inter-wrappers
   */
  virtual const Real & getPitch() const { return _assembly_pitch; }

  /**
   * Return side lengths of the assembly
   */
  virtual const Real & getSideX() const { return _assembly_side_x; }
  virtual const Real & getSideY() const { return _assembly_side_y; }

  /**
   * Return a signs for the cross flow given a inter-wrapper index and local neighbor index
   */
  virtual const Real & getCrossflowSign(unsigned int i_chan, unsigned int i_local) const = 0;

  /**
   * Return unheated length at entry
   */
  virtual const Real & getHeatedLengthEntry() const { return _unheated_length_entry; }

  /**
   * Return heated length
   */
  virtual const Real & getHeatedLength() const { return _heated_length; }

  /**
   * Return unheated length at exit
   */
  virtual const Real & getHeatedLengthExit() const { return _unheated_length_exit; }

  /**
   * Return a inter-wrapper index for a given physical point `p`
   */
  virtual unsigned int getSubchannelIndexFromPoint(const Point & p) const = 0;

  virtual unsigned int channelIndex(const Point & point) const = 0;

  /**
   * Return a pin index for a given physical point `p`
   */
  virtual unsigned int getPinIndexFromPoint(const Point & p) const = 0;

  virtual unsigned int pinIndex(const Point & p) const = 0;

  /**
   * Return the type of the inter-wrapper for given inter-wrapper index
   */
  virtual EChannelType getSubchannelType(unsigned int index) const = 0;

  /**
   * Return gap width for a given gap index
   */
  virtual Real getGapWidth(unsigned int gap_index) const = 0;

protected:
  /// unheated length of the fuel rod at the entry of the assembly
  Real _unheated_length_entry;
  /// heated length of the fuel rod
  Real _heated_length;
  /// unheated length of the fuel rod at the exit of the assembly
  Real _unheated_length_exit;
  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// axial form loss coefficient per computational cell
  std::vector<std::vector<Real>> _k_grid;
  /// Lateral form loss coefficient
  Real _kij;
  /// Distance between the neighbor fuel rods, pitch
  Real _assembly_pitch;
  /// fuel rod diameter
  Real _assembly_side_x;
  Real _assembly_side_y;
  /// number of axial cells
  unsigned int _n_cells;

public:
  static InputParameters validParams();

  /**
   * Generate the spacing in z-direction using heated and unteaded lengths
   */
  static void generateZGrid(Real unheated_length_entry,
                            Real heated_length,
                            Real unheated_length_exit,
                            unsigned int n_cells,
                            std::vector<Real> & z_grid);
};
