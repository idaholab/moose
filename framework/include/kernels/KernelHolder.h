#ifndef KERNELHOLDER_H
#define KERNELHOLDER_H

#include <vector>
#include <map>
#include <set>

#include "Kernel.h"


/**
 * Typedef to hide implementation details
 */
typedef std::vector<Kernel *>::iterator KernelIterator;


/**
 * Holds kernels and provides some services
 */
class KernelHolder
{
public:
  KernelHolder(MooseSystem &sys);
  virtual ~KernelHolder();

  KernelIterator activeKernelsBegin(THREAD_ID tid);
  KernelIterator activeKernelsEnd(THREAD_ID tid);

  KernelIterator blockKernelsBegin(THREAD_ID tid, unsigned int block_id);
  KernelIterator blockKernelsEnd(THREAD_ID tid, unsigned int block_id);

  bool activeKernelBlocks(std::set<subdomain_id_type> & set_buffer) const;

  void updateActiveKernels(THREAD_ID tid);

  std::vector<Kernel *> _kernels;
  std::vector<std::vector<Kernel *> > _active_kernels;
  std::vector<std::vector<Kernel *> > _all_kernels;
  std::vector<std::map<unsigned int, std::vector<Kernel *> > > _block_kernels;
  std::vector<std::map<unsigned int, std::vector<Kernel *> > > _all_block_kernels;

protected:
  MooseSystem &_moose_system;
};

#endif // KERNELHOLDER_H
