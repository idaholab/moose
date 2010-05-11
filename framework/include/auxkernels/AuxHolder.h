#ifndef AUXHOLDER_H
#define AUXHOLDER_H

#include <vector>
#include <map>
#include <string>

#include "AuxKernel.h"


/**
 * Typedef to hide implementation details
 */
typedef std::vector<AuxKernel *>::iterator AuxKernelIterator;


/**
 * TODO: describe me
 */
class AuxHolder
{
public:
  AuxHolder(MooseSystem &sys);
  virtual ~AuxHolder();

  std::vector<AuxKernel *>::iterator activeNodalAuxKernelsBegin(THREAD_ID tid);
  std::vector<AuxKernel *>::iterator activeNodalAuxKernelsEnd(THREAD_ID tid);

  std::vector<AuxKernel *>::iterator activeElementAuxKernelsBegin(THREAD_ID tid);
  std::vector<AuxKernel *>::iterator activeElementAuxKernelsEnd(THREAD_ID tid);

  std::vector<AuxKernel *>::iterator activeAuxBCsBegin(THREAD_ID tid, unsigned int boundary_id);
  std::vector<AuxKernel *>::iterator activeAuxBCsEnd(THREAD_ID tid, unsigned int boundary_id);

  std::vector<std::vector<AuxKernel *> > active_NodalAuxKernels;
  std::vector<std::vector<AuxKernel *> > active_ElementAuxKernels;

  std::vector<std::vector<AuxKernel *> > _aux_bcs;
  std::vector<std::map<unsigned int, std::vector<AuxKernel *> > > active_bcs;

protected:
  MooseSystem &_moose_system;
};

#endif // AUXHOLDER_H
