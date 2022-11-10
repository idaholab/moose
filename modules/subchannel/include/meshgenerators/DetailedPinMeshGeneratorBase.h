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

#include "MeshGenerator.h"

/**
 * Base class for generating fuel pins
 */
class DetailedPinMeshGeneratorBase : public MeshGenerator
{
public:
  DetailedPinMeshGeneratorBase(const InputParameters & parameters);

protected:
  void generatePin(std::unique_ptr<MeshBase> & mesh_base, const Point & center);

  /// unheated length of the fuel rod at the entry of the assembly
  Real _unheated_length_entry;
  /// heated length of the fuel rod
  Real _heated_length;
  /// unheated length of the fuel rod at the exit of the assembly
  Real _unheated_length_exit;
  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// Distance between the neighbor fuel rods, pitch
  Real _pitch;
  /// fuel rod diameter
  Real _rod_diameter;
  /// Number of cells in the axial direction
  unsigned int _n_cells;
  /// Subdomain ID used for the mesh block
  const unsigned int & _block_id;
  /// Number of radial parts
  const unsigned int & _num_radial_parts;
  /// Counter for element numbering
  dof_id_type _elem_id;

public:
  static InputParameters validParams();
};
