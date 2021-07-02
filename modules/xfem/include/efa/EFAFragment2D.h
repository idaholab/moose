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
class EFAElement2D;

class EFAFragment2D : public EFAFragment
{
public:
  EFAFragment2D(EFAElement2D * host,
                bool create_boundary_edges,
                const EFAElement2D * from_host,
                unsigned int frag_id = std::numeric_limits<unsigned int>::max());
  EFAFragment2D(EFAElement2D * host, const EFAFace * from_face);
  ~EFAFragment2D();

private:
  EFAElement2D * _host_elem;
  std::vector<EFAEdge *> _boundary_edges;
  std::vector<unsigned int> _frag_cut_plane_idx;
  std::vector<std::vector<EFANode *>> _frag_cut_plane_nodes;

public:
  // override pure virtual methods
  virtual void switchNode(EFANode * new_node, EFANode * old_node);
  virtual bool containsNode(EFANode * node) const;
  virtual unsigned int getNumCuts() const;
  virtual unsigned int getNumCutNodes() const;
  virtual std::set<EFANode *> getAllNodes() const;
  virtual bool isConnected(EFAFragment * other_fragment) const;
  virtual void removeInvalidEmbeddedNodes(std::map<unsigned int, EFANode *> & EmbeddedNodes);

  // EFAfragment2D specific methods
  void combineTipEdges();
  bool isEdgeInterior(unsigned int edge_id) const;
  std::vector<unsigned int> getInteriorEdgeID() const;
  bool isSecondaryInteriorEdge(unsigned int edge_id) const;
  unsigned int numEdges() const;
  EFAEdge * getEdge(unsigned int edge_id) const;
  void addEdge(EFAEdge * new_edge);
  std::set<EFANode *> getEdgeNodes(unsigned int edge_id) const;
  EFAElement2D * getHostElement() const;
  std::vector<EFAFragment2D *> split(std::map<unsigned int, EFANode *> & EmbeddedNodes);
  std::vector<EFAFragment2D *> split(std::map<unsigned int, EFANode *> & EmbeddedNodes,
                                     std::vector<unsigned int> frag_cut_plane_idx,
                                     std::vector<std::vector<EFANode *>> frag_cut_plane_nodes);
  bool IntersectSegmentWithCutLine(const std::vector<double> & segment_point1,
                                   const std::vector<double> & segment_point2,
                                   const std::vector<double> & cutting_line_point1,
                                   const std::vector<double> & cutting_line_point2,
                                   std::vector<double> & intersect_Point);
  double crossProduct2D(const std::vector<double> & point_a,
                        const std::vector<double> & point_b) const;
  double distanceBetweenPoints(const std::vector<double> & point_a,
                               const std::vector<double> & point_b) const;
  std::vector<EFAFragment2D *> split();
};
