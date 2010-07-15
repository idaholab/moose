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
  AuxWarehouse(MooseSystem &sys);
  virtual ~AuxWarehouse();

  AuxKernelIterator activeNodalAuxKernelsBegin(THREAD_ID tid);
  AuxKernelIterator activeNodalAuxKernelsEnd(THREAD_ID tid);

  AuxKernelIterator activeElementAuxKernelsBegin(THREAD_ID tid);
  AuxKernelIterator activeElementAuxKernelsEnd(THREAD_ID tid);

  AuxKernelIterator activeAuxBCsBegin(THREAD_ID tid, unsigned int boundary_id);
  AuxKernelIterator activeAuxBCsEnd(THREAD_ID tid, unsigned int boundary_id);

  std::list<AuxKernel *> getActiveNodalKernels(THREAD_ID tid);
  std::list<AuxKernel *> getActiveElementKernels(THREAD_ID tid);

  void setActiveNodalKernels(THREAD_ID tid, std::list<AuxKernel *> &auxs);
  void setActiveElementKernels(THREAD_ID tid, std::list<AuxKernel *> &auxs);

  void addBC(THREAD_ID tid, AuxKernel *aux);
  void addActiveBC(THREAD_ID tid, unsigned int boundary_id, AuxKernel *aux);

protected:
  std::vector<std::vector<AuxKernel *> > _active_nodal_aux_kernels;
  std::vector<std::vector<AuxKernel *> > _active_element_aux_kernels;

  std::vector<std::vector<AuxKernel *> > _aux_bcs;
  std::vector<std::map<unsigned int, std::vector<AuxKernel *> > > _active_bcs;

  MooseSystem &_moose_system;
};

#endif // AUXWAREHOUSE_H
