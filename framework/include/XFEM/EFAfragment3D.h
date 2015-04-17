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

#ifndef EFAFRAGMENT3D_H
#define EFAFRAGMENT3D_H

#include "EFAedge.h"
#include "EFAface.h"
#include "EFAfragment.h"

class EFAelement3D;

class EFAfragment3D : public EFAfragment
{
public:

  EFAfragment3D(EFAelement3D* host, bool create_faces, const EFAelement3D * from_host,
                unsigned int frag_id = std::numeric_limits<unsigned int>::max());
  ~EFAfragment3D();

private:

  EFAelement3D * _host_elem;
  std::vector<EFAface*> _faces;
  std::vector<std::vector<EFAface*> > _adjacent_face_ix;

public:
  // override pure virtual methods
  virtual void switchNode(EFAnode *new_node, EFAnode *old_node);
  virtual bool containsNode(EFAnode *node) const;
  virtual unsigned int get_num_cuts() const;
  virtual std::set<EFAnode*> get_all_nodes() const;
  virtual bool isConnected(EFAfragment *other_fragment) const;

  // EFAfragment3D specific methods
  void combine_tip_faces();
  bool is_face_interior(unsigned int face_id) const;
  std::vector<unsigned int> get_interior_face_id() const;
  bool isThirdInteriorFace(unsigned int face_id) const;
  unsigned int num_faces() const;
  EFAface* get_face(unsigned int face_id) const;
  unsigned int get_face_id(EFAface* face) const;
  void add_face(EFAface* new_face);
  std::set<EFAnode*> get_face_nodes(unsigned int face_id) const;
  EFAelement3D * get_host() const;
  std::vector<EFAfragment3D*> split();
  void create_adjacent_face_ix();
  EFAface* get_adjacent_face(unsigned int face_id, unsigned int edge_id) const;
  void remove_invalid_embedded(std::map<unsigned int, EFAnode*> &EmbeddedNodes);
  void remove_embedded_node(EFAnode* emb_node);
  bool hasFaceWithOneCut() const;

private:

  EFAfragment3D* connect_subfaces(EFAface* start_face, unsigned int startOldFaceID,
                                  std::vector<std::vector<EFAface*> > &subfaces);
  EFAedge* lonelyEdgeOnFace(unsigned int face_id) const;
  void combine_two_faces(unsigned int face_id1, unsigned int face_id2, const EFAface* elem_face);
};

#endif
