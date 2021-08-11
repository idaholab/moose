//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeometricCutUserObject.h"

class ComboCutUserObject : public GeometricCutUserObject
{
public:
  static InputParameters validParams();

  ComboCutUserObject(const InputParameters & parameters);

  /**
   * Loop over all the provided GeometricCutUserObjects, fill the data structures based on each
   * cut that wants to cut this 2D element.
   * @param elem      Pointer to the libMesh element to be considered for cutting
   * @param cut_edges Data structure filled with information about edges to be cut
   * @param cut_nodes Data structure filled with information about nodes to be cut
   * @return bool     true if element is to be cut
   */
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutEdge> & cut_edges,
                                    std::vector<Xfem::CutNode> & cut_nodes) const override;

  /**
   * Loop over all the provided GeometricCutUserObjects, fill the data structures based on each
   * cut that wants to cut this 3D element.
   * @param elem      Pointer to the libMesh element to be considered for cutting
   * @param cut_edges Data structure filled with information about edges to be cut
   * @param cut_nodes Data structure filled with information about nodes to be cut
   * @return bool     true if element is to be cut
   */
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutFace> & cut_faces) const override;

  /**
   * Loop over all the provided GeometricCutUserObjects, fill the data structures based on each
   * cut that wants to cut this fragment of 2D element.
   * @param elem      Pointer to the libMesh element to be considered for cutting
   * @param cut_edges Data structure filled with information about edges to be cut
   * @param cut_nodes Data structure filled with information about nodes to be cut
   * @return bool     true if element is to be cut
   */
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<Xfem::CutEdge> & cut_edges) const override;

  /**
   * Loop over all the provided GeometricCutUserObjects, fill the data structures based on each
   * cut that wants to cut this fragment of 3D element.
   * @param elem      Pointer to the libMesh element to be considered for cutting
   * @param cut_edges Data structure filled with information about edges to be cut
   * @param cut_nodes Data structure filled with information about nodes to be cut
   * @return bool     true if element is to be cut
   */
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<Xfem::CutFace> & cut_faces) const override;

  /// The ComboCutUserObject shouldn't be used to provided crack front data.
  virtual const std::vector<Point> getCrackFrontPoints(unsigned int) const override
  {
    mooseError("getCrackFrontPoints() is not implemented for this object.");
  }

  virtual const std::vector<RealVectorValue> getCrackPlaneNormals(unsigned int) const override
  {
    mooseError("getCrackPlaneNormals() is not implemented for this object.");
  }

  // Get the CutSubdomainID for each provided geometric cut, then lookup the resulting
  // combo CutSubdomainID in the user-specified dictionary.
  virtual CutSubdomainID getCutSubdomainID(const Node * node) const override;

protected:
private:
  /// Helper function to build the dictionary for composite CutSubdomainID look-up
  void buildMap();

  /// Vector of names of GeometricCutUserObjects to be combined
  const std::vector<UserObjectName> _cut_names;

  /// Number of geometric cuts to be combined
  unsigned int _num_cuts;

  /// Vector of points to the GeometricCutUserObjects to be combined
  std::vector<const GeometricCutUserObject *> _cuts;

  /// Keys read from the input file, to be parsed by buildMap()
  const std::vector<std::vector<CutSubdomainID>> _keys;

  /// Values read from the input file, to be parsed by buildMap()
  const std::vector<CutSubdomainID> _vals;

  /**
   * The dictionary for composite CutSubdomainID look-up. Keys are combinations of the
   * CutSubdomainIDs, values are the composite CutSubdomainIDs.
   */
  std::map<std::vector<CutSubdomainID>, CutSubdomainID> _combo_ids;
};
