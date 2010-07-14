#ifndef FUNCTIONINTERFACE_H
#define FUNCTIONINTERFACE_H

#include "MooseObject.h"

class Function;
class FunctionWarehouse;

/**
 * Inherit from this class at a very low level to make the getFunction method
 * available.
 */
class FunctionInterface
{
public:
  /**
   * @param func_warehouse Reference to the FunctionWarehouse stored by MooseSystem
   * @param tid The thread id used by this object, used by the warehouse
   */
  FunctionInterface(FunctionWarehouse & func_warehouse, InputParameters params, THREAD_ID tid);

  Function & getFunction(std::string name);

private:
  //prefixed all member data with _func to prevent future Multiple Inheritance
  //issues. The compiler will complain even though it's private data
  InputParameters _func_params;
  FunctionWarehouse & _func_warehouse;

  /**
   * Private copy of the tid so getFunction() knows which tid to use to retrieve
   * the Function object. It is not named _tid because then the compiler doesn't
   * like sub classes of PDEBase using _tid because it is ambiguous (even though
   * _tid would be private here for some reason).
   */
  THREAD_ID _func_tid;
};

#endif //FUNCTIONINTERFACE_H
