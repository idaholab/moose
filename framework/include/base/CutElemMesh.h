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

#define CutElemMeshError(msg) {std::cout<<"CutElemMesh ERROR: "<<msg<<std::endl; exit(1);}

class CutElemMesh
{
  public:
  class element_t;

  enum N_CATEGORY
  {
    N_CATEGORY_PERMANENT,
    N_CATEGORY_TEMP,
    N_CATEGORY_EMBEDDED,
    N_CATEGORY_LOCAL_INDEX
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

  class faceNode
  {
    public:
    faceNode(node_t* node, double xi, double eta);
    faceNode(const faceNode & other_face_node);

    ~faceNode();

    node_t * get_node();
    double get_para_coords(unsigned int i);
    void switchNode(node_t* new_old, node_t* old_node);

    private:

    node_t * _node;
    double _xi;
    double _eta;
  };

  class edge_t
  {
    public:
    edge_t(node_t * node1, node_t * node2);
    edge_t(const edge_t & other_edge);
    edge_t(const edge_t & other_edge, element_t* host, bool convert_to_local);

    // the destructor must delete any local nodes
    ~edge_t();

    bool equivalent(const edge_t & other) const; // compares end nodes and embedded node
    bool isOverlapping(const edge_t & other) const; // only compares end nodes
    bool containsEdge(edge_t & other);

//    bool operator < (const edge_t & other) const;

    void add_intersection(double position, node_t * embedded_node_tmp, node_t * from_node);
    node_t * get_node(unsigned int index);

    bool has_intersection();
    bool has_intersection_at_position(double position, node_t * from_node);
    double get_intersection(node_t * from_node);

    node_t * get_embedded_node();
    void consistency_check();
    void switchNode(node_t *new_node, node_t *old_node);
    bool containsNode(node_t *node);
    bool getNodeMasters(node_t* node, std::vector<node_t*> &master_nodes, std::vector<double> &master_weights);
    bool is_interior_edge();
    void remove_embedded_node();

    private:
    node_t * edge_node1;
    node_t * edge_node2;
    node_t * embedded_node;
    double intersection_x;
  };

  class fragment_t
  {
    public:
    fragment_t(element_t * host,
               bool create_boundary_edges,
               const element_t * from_host,
               unsigned int fragment_copy_index = std::numeric_limits<unsigned int>::max());

    //Construct a fragment from another fragment.  If convert_to_local is true,
    //convert the nodes to local nodes, otherwise convert them to global nodes.
    fragment_t(const fragment_t & other_frag,
               element_t * host,
               bool convert_to_local);

    //The destructor must delete any local nodes
    ~fragment_t();

    void switchNode(node_t *new_node, node_t *old_node);
    bool containsNode(node_t *node);
    bool isConnected(fragment_t &other_fragment);
    void combine_tip_edges();
    std::vector<fragment_t*> split();
    std::vector<node_t*> commonNodesWithEdge(edge_t & other_edge);
    unsigned int get_num_cuts();
    element_t* get_host();

//    std::vector< node_t*> boundary_nodes;
    std::vector< edge_t*> boundary_edges;

    private:
    element_t * host_elem;
  };

  class element_t
  {
    public:

    element_t(unsigned int eid);
    element_t(const element_t * from_elem, bool convert_to_local);

    ~element_t();

    void
    createEdges();

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
    bool shouldDuplicateCrackTipSplitElem();

    //Given a global node, create a new local node
    node_t * create_local_node_from_global_node(const node_t * global_node) const;

    //Given a local node, find the global node corresponding to that node
    node_t * get_global_node_from_local_node(const node_t * local_node) const;

    //Given a node_t, find the element edge or fragment edge that contains it
    //Return its master nodes and weights
    void getMasterInfo(node_t* node, std::vector<node_t*> &master_nodes,
                       std::vector<double> &master_weights) const;

    //Get the local node index of node
    unsigned int getLocalNodeIndex(node_t * node) const;

    //Given a element edge ID, get the corresponding fragment ID
    bool getFragmentEdgeID(unsigned int elem_edge_id, unsigned int &frag_id, unsigned int &frag_edge_id);

    //add an interior edge
    bool is_edge_phantom(unsigned int edge_id);

    //check if the element has only one fragment which has tip edges
    bool frag_has_tip_edges();

    //get the parametric coords of an embedded node
    bool getEmbeddedNodeParaCoor(node_t* embedded_node, std::vector<double> &para_coor);
    unsigned int getNumInteriorNodes();
    unsigned int get_num_cuts();

    //id
    unsigned int id;
    unsigned int num_nodes;
    unsigned int num_edges;
    //array of nodes
    std::vector<node_t*> nodes;
    //list of cut edges
    std::vector<edge_t*> edges;
    //list of interior embedded nodes
    std::vector<faceNode*> interior_nodes;
    //parent
    element_t * parent;
    //neighbors on edge
    std::vector<std::vector<element_t*> >edge_neighbors;
    //fragments
    std::vector< fragment_t*> fragments;
    //set of children
    std::vector< element_t* > children;
    //special case at crack tip
    bool crack_tip_split_element;
    std::vector<unsigned int> crack_tip_neighbors;

    private:

    void mapParaCoorFrom1Dto2D(unsigned int edge_id, unsigned int num_edges,
                               double xi_1d, std::vector<double> &para_coor);
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
  void addFragEdgeIntersection(unsigned int elemid, unsigned int frag_edge_id, double position);

  void updatePhysicalLinksAndFragments();
  void physicalLinkAndFragmentSanityCheck(element_t *currElem);

  void updateTopology(bool mergeUncutVirtualEdges=true);
  void reset();
  void clearAncestry();
  void restoreFragmentInfo(CutElemMesh::element_t * const elem, CutElemMesh::element_t & from_elem);
  void restoreEdgeIntersections(CutElemMesh::element_t * const elem, CutElemMesh::element_t & from_elem);

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
  void duplicateInteriorEmbeddedNode(element_t* currElem);
  bool should_duplicate_for_crack_tip(element_t* currElem);

  void sanityCheck();
  void findCrackTipElements();
  void printMesh();
  void error(const std::string &error_string);

  const std::vector<element_t*> &getChildElements(){return ChildElements;};
  const std::vector<element_t*> &getParentElements(){return ParentElements;};
  const std::vector<node_t*> &getNewNodes(){return NewNodes;};
  const std::set<element_t*> &getCrackTipElements(){return CrackTipElements;};
  element_t* getElemByID(unsigned int id);
  unsigned int getElemIdByNodes(unsigned int * node_id);

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
  std::map< node_t*, std::set< element_t *> > InverseConnectivityMap;
};

#endif // #ifndef CUTELEM_MESH_H
