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

#ifndef ELEMENTFRAGMENTALGORITHM_H
#define ELEMENTFRAGMENTALGORITHM_H

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
#include "EFAelement2D.h"
#include "EFAelement3D.h"

class ElementFragmentAlgorithm
{
public:

  /**
   * Constructor
   **/
  ElementFragmentAlgorithm();

  ~ElementFragmentAlgorithm();

private:
  //unsigned int MaxElemId;
  unsigned int _mesh_dim;
  std::map< unsigned int, EFAnode*> _permanent_nodes;
  std::map< unsigned int, EFAnode*> _embedded_nodes;
  std::map< unsigned int, EFAnode*> _temp_nodes;
  std::map< unsigned int, EFAelement*> _elements;
//  std::map< std::set< EFAnode* >, std::set< EFAelement* > > _merged_edge_map;
  std::set< EFAelement*> _crack_tip_elements;
  std::vector< EFAnode* > _new_nodes;
  std::vector< EFAelement* > _child_elements;
  std::vector< EFAelement* > _parent_elements;
  std::map< EFAnode*, std::set< EFAelement *> > _inverse_connectivity;

public:

  unsigned int add2DElements( std::vector< std::vector<unsigned int> > &quads );
  EFAelement* add2DElement( std::vector<unsigned int> quad, unsigned int id );
  EFAelement* add3DElement( std::vector<unsigned int> quad, unsigned int id );
  void set_dimension(unsigned int ndm);

  void updateEdgeNeighbors();
  void initCrackTipTopology();
  void addElemEdgeIntersection(unsigned int elemid, unsigned int edgeid, double position);
  void addFragEdgeIntersection(unsigned int elemid, unsigned int frag_edge_id, double position);
  void addElemFaceIntersection(unsigned int elemid, unsigned int faceid,
                               std::vector<unsigned int> edgeid, std::vector<double> position);

  void updatePhysicalLinksAndFragments();

  void updateTopology(bool mergeUncutVirtualEdges=true);
  void reset();
  void clearAncestry();
  void restoreFragmentInfo(EFAelement * const elem, const EFAelement * const from_elem);

  void createChildElements();
  void connectFragments(bool mergeUncutVirtualEdges);

  void sanityCheck();
  void findCrackTipElements();
  void printMesh();
  void error(const std::string &error_string);

  const std::vector<EFAelement*> &getChildElements(){return _child_elements;};
  const std::vector<EFAelement*> &getParentElements(){return _parent_elements;};
  const std::vector<EFAnode*> &getNewNodes(){return _new_nodes;};
  const std::set<EFAelement*> &getCrackTipElements(){return _crack_tip_elements;};
  EFAelement* getElemByID(unsigned int id);
  unsigned int getElemIdByNodes(unsigned int * node_id);
};

#endif // #ifndef CUTELEM_MESH_H
