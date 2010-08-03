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
  FunctionWarehouse();
  virtual ~FunctionWarehouse();

  /**
   * Access to iterators.
   */
  FunctionIterator activeFunctionsBegin();
  FunctionIterator activeFunctionsEnd();

  /**
   * Get function by name and tid.
   */
  Function & getFunction(const std::string & fname);

  /**
   * Add function by name and tid.
   */
  void addFunction(const std::string & fname, Function *func);

protected:
  std::map<std::string, Function *> _functions;
};

#endif // FUNCTIONWAREHOUSE_H
