/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef CUTELEM_MESH_H
#define CUTELEM_MESH_H

#include <cstddef>
#include <iostream>
#include <sstream>

#include <vector>
#include <map>
#include <set>
#include <limits>

#include "EFAnode.h"
#include "FaceNode.h"
#include "EFAedge.h"
#include "EFAfragment.h"
#include "EFAelement.h"

class CutElemMesh
{
public:

  /**
   * Constructor
   **/
  CutElemMesh();

  ~CutElemMesh();

  template <typename T> static bool deleteFromMap(std::map<unsigned int, T*> &theVector, T* elemToDelete);
  template <typename T> static unsigned int getNewID(std::map<unsigned int, T*> &theMap);

  unsigned int addElements( std::vector< std::vector<unsigned int> > &quads );
  EFAelement* addElement( std::vector<unsigned int> quad, unsigned int id );

  void updateEdgeNeighbors();
  void initCrackTipTopology();
  void addEdgeIntersection( unsigned int elemid, unsigned int edgeid, double position );
  void addEdgeIntersection( EFAelement * elem, unsigned int edgeid, double position, EFAnode * embedded_node = NULL );
  void addFragEdgeIntersection(unsigned int elemid, unsigned int frag_edge_id, double position);

  void updatePhysicalLinksAndFragments();
  void physicalLinkAndFragmentSanityCheck(EFAelement *currElem);

  void updateTopology(bool mergeUncutVirtualEdges=true);
  void reset();
  void clearAncestry();
  void restoreFragmentInfo(EFAelement * const elem, EFAelement * const from_elem);
  void restoreEdgeIntersections(EFAelement * const elem, EFAelement * const from_elem);

  void createChildElements();
  void connectFragments( bool mergeUncutVirtualEdges);
  void mergeNodes(EFAnode* &childNode,
                  EFAnode* &childOfNeighborNode,
                  EFAelement* childElem,
                  EFAelement* childOfNeighborElem);

  void addToMergedEdgeMap(EFAnode* node1,
                          EFAnode* node2,
                          EFAelement* elem1,
                          EFAelement* elem2);

  void duplicateEmbeddedNode(EFAelement* currElem,
                             EFAelement* neighborElem,
                             unsigned int edgeID,
                             unsigned int neighborEdgeID);

  void duplicateEmbeddedNode(EFAelement* currElem,
                             unsigned int edgeID);
  void duplicateInteriorEmbeddedNode(EFAelement* currElem);
  bool should_duplicate_for_crack_tip(EFAelement* currElem);

  void sanityCheck();
  void findCrackTipElements();
  void printMesh();
  void error(const std::string &error_string);

  const std::vector<EFAelement*> &getChildElements(){return ChildElements;};
  const std::vector<EFAelement*> &getParentElements(){return ParentElements;};
  const std::vector<EFAnode*> &getNewNodes(){return NewNodes;};
  const std::set<EFAelement*> &getCrackTipElements(){return CrackTipElements;};
  EFAelement* getElemByID(unsigned int id);
  unsigned int getElemIdByNodes(unsigned int * node_id);

private:
  //unsigned int MaxElemId;
  std::map< unsigned int, EFAnode*> PermanentNodes;
  std::map< unsigned int, EFAnode*> EmbeddedNodes;
  std::map< unsigned int, EFAnode*> TempNodes;
  std::map< unsigned int, EFAelement*> Elements;
  std::map< std::set< EFAnode* >, std::set< EFAelement* > > MergedEdgeMap;
  std::set< EFAelement*> CrackTipElements;
  std::vector< EFAnode* > NewNodes;
  std::vector< EFAelement* > ChildElements;
  std::vector< EFAelement* > ParentElements;
  std::map< EFAnode*, std::set< EFAelement *> > InverseConnectivityMap;

private:

  template <class T> 
  unsigned int num_common_elems(std::set<T> &v1, std::vector<T> &v2);
};

#endif // #ifndef CUTELEM_MESH_H
