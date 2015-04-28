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

#include "XFEMCutElem.h"

XFEMCutElem::XFEMCutElem(Elem* elem, unsigned int n_qpoints):
  _n_nodes(elem->n_nodes()),
  _n_qpoints(n_qpoints),
  _nodes(_n_nodes,NULL)
{
  for (unsigned int i = 0; i < _n_nodes; ++i)
    _nodes[i] = elem->get_node(i);
  _elem_volume = elem->volume();
}

XFEMCutElem::~XFEMCutElem()
{
}

Real
XFEMCutElem::get_physical_volfrac() const
{
  return _physical_volfrac;
}

Real
XFEMCutElem::get_mf_weights(unsigned int i_qp) const
{
  return _new_weights[i_qp];
}
