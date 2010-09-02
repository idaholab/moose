/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef AUXWAREHOUSE_H
#define AUXWAREHOUSE_H

#include <vector>
#include <map>
#include <string>
#include <list>

#include "AuxKernel.h"


/**
 * Typedef to hide implementation details
 */
typedef std::vector<AuxKernel *>::iterator AuxKernelIterator;


/**
 * TODO: describe me
 */
class AuxWarehouse
{
public:
  AuxWarehouse();
  virtual ~AuxWarehouse();

  AuxKernelIterator activeNodalAuxKernelsBegin();
  AuxKernelIterator activeNodalAuxKernelsEnd();

  AuxKernelIterator activeElementAuxKernelsBegin();
  AuxKernelIterator activeElementAuxKernelsEnd();

  AuxKernelIterator activeAuxBCsBegin(unsigned int boundary_id);
  AuxKernelIterator activeAuxBCsEnd(unsigned int boundary_id);

  std::list<AuxKernel *> getActiveNodalKernels();
  std::list<AuxKernel *> getActiveElementKernels();

  void setActiveNodalKernels(std::list<AuxKernel *> &auxs);
  void setActiveElementKernels(std::list<AuxKernel *> &auxs);

  void addBC(AuxKernel *aux);
  void addActiveBC(unsigned int boundary_id, AuxKernel *aux);

protected:
  std::vector<AuxKernel *> _active_nodal_aux_kernels;
  std::vector<AuxKernel *> _active_element_aux_kernels;

  std::vector<AuxKernel *> _aux_bcs;
  std::map<unsigned int, std::vector<AuxKernel *> > _active_bcs;
};

#endif // AUXWAREHOUSE_H
