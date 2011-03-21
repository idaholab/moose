#ifndef KERNELWAREHOUSE_H
#define KERNELWAREHOUSE_H

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
class KernelWarehouse
{
public:
  KernelWarehouse();
  virtual ~KernelWarehouse();

  KernelIterator allKernelsBegin();
  KernelIterator allKernelsEnd();

  KernelIterator activeKernelsBegin();
  KernelIterator activeKernelsEnd();
  
  void addKernel(Kernel *kernel, const std::set<unsigned int> & block_ids);

  void updateActiveKernels(Real t, Real dt, unsigned int subdomain_id);

 /**
   * This returns a boolean to indicate whether this warehouse contains kernels
   * representing all of the subdomains, if not then the supplied set is filled in
   * with the complete set of subdomains represented which may or may not represent
   * the entire domain.
   */
  bool subdomains_covered(std::set<unsigned int> & return_set) const;
  
protected:
  std::vector<Kernel *> _active_kernels;                                /// Kernels active on a block and in specified time
  std::vector<Kernel *> _all_kernels;                                   /// All instances of kernels
  std::vector<Kernel *> _global_kernels;                                /// Kernels that live everywhere (on the whole domain)
  std::map<unsigned int, std::vector<Kernel *> > _block_kernels;        /// Kernels that live on a specified block
};

#endif // KERNELWAREHOUSE_H
