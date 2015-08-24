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

#ifndef XFEMCUTELEM2D_H
#define XFEMCUTELEM2D_H

#include "XFEMCutElem.h"

using namespace libMesh;

class XFEMCutElem2D : public XFEMCutElem
{
public:

  XFEMCutElem2D(Elem* elem, const EFAelement2D * const CEMelem, unsigned int n_qpoints);
  ~XFEMCutElem2D();

private:

  EFAelement2D _efa_elem2d; // 2D EFAelement
  virtual Point get_node_coords(EFAnode* node, MeshBase* displaced_mesh = NULL) const;

public:

  virtual void calc_physical_volfrac();
  virtual void calc_mf_weights();
  virtual Point get_origin(unsigned int plane_id, MeshBase* displaced_mesh=NULL) const;
  virtual Point get_normal(unsigned int plane_id, MeshBase* displaced_mesh=NULL) const;
  virtual void get_frag_faces(std::vector<std::vector<Point> > &frag_faces, MeshBase* displaced_mesh=NULL) const;
  virtual const EFAelement * get_efa_elem() const;
  virtual unsigned int num_cut_planes() const;

private:

  void new_weight_mf(unsigned int nen, unsigned int nqp, std::vector<Point> &elem_nodes,
                     std::vector<std::vector<Real> > &wsg);
  void partial_gauss(unsigned int nen, std::vector<std::vector<Real> > &tsg);
  void solve_mf(unsigned int nen, unsigned int nqp, std::vector<Point> &elem_nodes,
                std::vector<std::vector<Real> > &tsg, std::vector<std::vector<Real> > &wsg);

};

#endif
