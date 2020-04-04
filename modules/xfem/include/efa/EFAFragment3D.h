//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "EFAFragment.h"

#include <limits>

class EFAEdge;
class EFAFace;
class EFAElement3D;

class EFAFragment3D : public EFAFragment
{
public:
  EFAFragment3D(EFAElement3D * host,
                bool create_faces,
                const EFAElement3D * from_host,
                unsigned int frag_id = std::numeric_limits<unsigned int>::max());
  ~EFAFragment3D();

private:
  EFAElement3D * _host_elem;
  std::vector<EFAFace *> _faces;
  std::vector<std::vector<EFAFace *>> _faces_adjacent_to_faces;

public:
  // override pure virtual methods
  virtual void switchNode(EFANode * new_node, EFANode * old_node);
  virtual bool containsNode(EFANode * node) const;
  virtual unsigned int getNumCuts() const;
  virtual unsigned int getNumCutNodes() const;
  virtual std::set<EFANode *> getAllNodes() const;
  virtual bool isConnected(EFAFragment * other_fragment) const;
  virtual bool isEdgeConnected(EFAFragment * other_fragment) const;
  virtual void removeInvalidEmbeddedNodes(std::map<unsigned int, EFANode *> & EmbeddedNodes);

  // EFAfragment3D specific methods
  void combine_tip_faces();
  bool isFaceInterior(unsigned int face_id) const;
  std::vector<unsigned int> get_interior_face_id() const;
  bool isThirdInteriorFace(unsigned int face_id) const;
  unsigned int numFaces() const;
  EFAFace * getFace(unsigned int face_id) const;
  unsigned int getFaceID(EFAFace * face) const;
  void addFace(EFAFace * new_face);
  std::set<EFANode *> getFaceNodes(unsigned int face_id) const;
  EFAElement3D * getHostElement() const;
  std::vector<EFAFragment3D *> split();
  void findFacesAdjacentToFaces();
  EFAFace * getAdjacentFace(unsigned int face_id, unsigned int edge_id) const;
  void removeEmbeddedNode(EFANode * emb_node);
  bool hasFaceWithOneCut() const;
  void getNodeInfo(std::vector<std::vector<unsigned int>> & face_node_indices,
                   std::vector<EFANode *> & nodes) const;

private:
  EFAFragment3D * connectSubfaces(EFAFace * start_face,
                                  unsigned int startOldFaceID,
                                  std::vector<std::vector<EFAFace *>> & subfaces);
  EFAEdge * loneEdgeOnFace(unsigned int face_id) const;
  void combine_two_faces(unsigned int face_id1, unsigned int face_id2, const EFAFace * elem_face);
};
