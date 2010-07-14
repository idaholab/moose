#ifndef FUNCTIONWAREHOUSE_H
#define FUNCTIONWAREHOUSE_H

#include <vector>
#include <map>

#include "Function.h"

/**
 * Typedef to hide implementation details
 */
typedef std::map<std::string, Function *>::iterator FunctionIterator;


/**
 * Holds Functions for kernels, bcs, etc to use.
 */
class FunctionWarehouse
{
public:
  FunctionWarehouse(MooseSystem &sys);
  virtual ~FunctionWarehouse();

  /**
   * Access to iterators.
   */
  FunctionIterator activeFunctionsBegin(THREAD_ID tid);
  FunctionIterator activeFunctionsEnd(THREAD_ID tid);

  /**
   * Get function by name.
   */
  Function & getFunction(THREAD_ID tid, std::string fname);

  /**
   * Add function by name.
   */
  void addFunction(THREAD_ID tid, std::string fname, Function *func);

protected:
  std::vector<std::map<std::string, Function *> > _functions;

  MooseSystem &_moose_system;
};

#endif // FUNCTIONWAREHOUSE_H
