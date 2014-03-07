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

#define CutElemMeshError(msg) std::cout<<"CutElemMesh ERROR: "<<msg<<std::endl; exit(1);

class CutElemMesh
{
  public:

  enum N_CATEGORY
  {
    N_CATEGORY_PERMANENT,
    N_CATEGORY_TEMP,
    N_CATEGORY_EMBEDDED
  };

  enum FRAG_NODE_CATEGORY
  {
    FRAG_NODE_EMBEDDED,
    FRAG_NODE_LOCAL_INDEX
  };

  class node_t
  {
    public:

    node_t(unsigned int nid, N_CATEGORY ncat, node_t* nparent=NULL):
      category(ncat),
      id(nid),
      parent(nparent)
    {};
    N_CATEGORY category;
    unsigned int id;
    node_t* parent;
    std::string id_cat_str()
    {
      std::ostringstream s;
      s << id;
      if (category == N_CATEGORY_EMBEDDED)
        s<<"e";
      else if (category == N_CATEGORY_TEMP)
        s<<"t";
      else
        s<<" ";
      return s.str();
    };
  };

  class element_t
  {
    public:

    element_t(unsigned int eid):
      id(eid),
      num_nodes(4),
      num_edges(4),
      nodes(num_nodes,NULL),
      local_edge_has_intersection(num_edges,false),
      embedded_nodes_on_edge(num_edges,NULL),
      intersection_x(num_edges,-1.0),
      parent(NULL),
      edge_neighbors(num_edges,std::vector<element_t*>(1,NULL)),
      nb_frags(0),
      crack_tip_element(false)
    {};

    ~element_t(){};

    void
    switchNode(node_t *new_node,
               node_t *old_node,
               bool descend_to_parent = true);

    void
    switchEmbeddedNode(node_t *new_node,
                       node_t *old_node);

    bool is_partial();

    //Check if two elements that share a common edge (defined by edge end nodes) overlay each other
    //by looking at the ordering of the nodes.
    bool overlays_elem(node_t* other_edge_node1, node_t* other_edge_node2);

    bool overlays_elem(element_t* other_elem);

    //Get the index of the specifed element in the edge_neighbors vector
    unsigned int get_neighbor_index(element_t * neighbor_elem);

    //Find out what side the specified element is on, and add it as a crack tip neighbor
    //element for that side.
    void add_crack_tip_neighbor(element_t * neighbor_elem);

    //Determine whether the current element is a crack tip element for which the crack will
    //extend into the next element.
    bool will_crack_tip_extend(std::vector<unsigned int> &split_neighbors);

    //Get the nodes on the specified edge and return in the edge_nodes vector
    void get_nodes_on_edge(unsigned int edge_idx, std::vector<node_t*> &edge_nodes);

    //Create a set of all non-physical nodes in the current element
    void get_non_physical_nodes(std::set<node_t*> &non_physical_nodes);

    //Determine whether element at crack tip should be duplicated.  It should be duplicated
    //if the crack will extend into the next element, or if it has a non-physical node
    //connected to a face where a crack terminates, but will extend.
    bool should_duplicate_for_crack_tip();

    //id
    unsigned int id;
    unsigned int num_nodes;
    unsigned int num_edges;
    //array of nodes
    std::vector<node_t*> nodes;
    //list of cut edges
    std::vector<bool> local_edge_has_intersection;
    std::vector<node_t*> embedded_nodes_on_edge;
    std::vector<double> intersection_x;
    //parent
    element_t * parent;
    //neighbors on edge
    std::vector<std::vector<element_t*> >edge_neighbors;
    //no of fragments
    unsigned int nb_frags;
    //interior links
    std::vector< std::vector< node_t*> > interior_links;
    //set of children
    std::vector< element_t* > children;
    //special case at crack tip
    bool crack_tip_element;
    std::vector<unsigned int> crack_tip_neighbors;
  };

  public:

  /**
   * Constructor
   **/
  CutElemMesh();

  ~CutElemMesh();

  template <typename T> static bool deleteFromMap(std::map<unsigned int, T*> &theVector, T* elemToDelete);
  template <typename T> static unsigned int getNewID(std::map<unsigned int, T*> &theMap);

  unsigned int addElements( std::vector< std::vector<unsigned int> > &quads );
  CutElemMesh::element_t* addElement( std::vector<unsigned int> quad, unsigned int id );

  void updateEdgeNeighbors();
  void initCrackTipTopology();
  void addEdgeIntersection( unsigned int elemid, unsigned int edgeid, double position );
  void addEdgeIntersection( element_t * elem, unsigned int edgeid, double position, node_t * embedded_node = NULL );

  void updatePhysicalLinksAndFragmentsOld();
  void updatePhysicalLinksAndFragments();
  void physicalLinkAndFragmentSanityCheck(element_t *currElem);

  void updateTopology(bool mergeUncutVirtualEdges=true);
  void reset();
  void clearAncestry();
  void restoreFragmentInfo(CutElemMesh::element_t * const elem,
                           const std::vector<std::pair<FRAG_NODE_CATEGORY, unsigned int> > &interior_link);
  void restoreEdgeIntersections(CutElemMesh::element_t * const elem,
                                const std::vector<bool> &local_edge_has_intersection,
                                const std::vector<CutElemMesh::node_t*> &embedded_nodes_on_edge,
                                const std::vector<double> &intersection_x);

  void createChildElements();
  void connectFragments( bool mergeUncutVirtualEdges);
  void mergeNodes(node_t* &childNode,
                  node_t* &childOfNeighborNode,
                  element_t* childElem,
                  element_t* childOfNeighborElem);

  void addToMergedEdgeMap(node_t* node1,
                          node_t* node2,
                          element_t* elem1,
                          element_t* elem2);

  void duplicateEmbeddedNode(element_t* currElem,
                             element_t* neighborElem,
                             unsigned int edgeID,
                             unsigned int neighborEdgeID);

  void duplicateEmbeddedNode(element_t* currElem,
                             unsigned int edgeID);

  void sanityCheck();
  void findCrackTipElements();
  void printMesh();
  void error(const std::string &error_string);

  const std::vector<element_t*> &getChildElements(){return ChildElements;};
  const std::vector<element_t*> &getParentElements(){return ParentElements;};
  const std::vector<node_t*> &getNewNodes(){return NewNodes;};
  const std::set<element_t*> &getCrackTipElements(){return CrackTipElements;};
  element_t* getElemByID(unsigned int it);

  private:
  //unsigned int MaxElemId;
  std::map< unsigned int, node_t*> PermanentNodes;
  std::map< unsigned int, node_t*> EmbeddedNodes;
  std::map< unsigned int, node_t*> TempNodes;
  std::map< unsigned int, element_t*> Elements;
  std::map< std::set< node_t* >, std::set< element_t* > > MergedEdgeMap;
  std::set< element_t*> CrackTipElements;
  std::vector< node_t* > NewNodes;
  std::vector< element_t* > ChildElements;
  std::vector< element_t* > ParentElements;

};

#endif // #ifndef CUTELEM_MESH_H
