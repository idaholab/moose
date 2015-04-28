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

#ifndef XFEMCUTELEM3D_H
#define XFEMCUTELEM3D_H

#include "XFEMCutElem.h"

using namespace libMesh;

class XFEMCutElem3D : public XFEMCutElem
{
public:
  XFEMCutElem3D(Elem* elem, const EFAelement3D * const CEMelem, unsigned int n_qpoints);
  ~XFEMCutElem3D();

private:

  EFAelement3D _efa_elem3d; // 3D EFAelement
  virtual Point get_node_coords(EFAnode* node, MeshBase* displaced_mesh = NULL) const;

public:
  virtual void calc_physical_volfrac();
  virtual void calc_mf_weights();
  virtual Point get_origin(unsigned int plane_id, MeshBase* displaced_mesh=NULL) const;
  virtual Point get_normal(unsigned int plane_id, MeshBase* displaced_mesh=NULL) const;
  virtual const EFAelement * get_efa_elem() const;
  virtual unsigned int num_cut_planes() const;

private:
  double polyhedron_volume_3d(double coord[], int order_max, int face_num,
                              int node[], int node_num, int order[]) const;
  void i4vec_zero(int n, int a[]) const;
};

#endif
