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

#ifndef FUNCTIONINTERFACE_H
#define FUNCTIONINTERFACE_H

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
   * @param params The parameters used by the object being instantiated. This
   *        class needs them so it can get the function named in the input file,
   *        but the object calling getFunction only needs to use the name on the
   *        left hand side of the statement "function = func_name"
   * @param tid The thread id used by this object, used by the warehouse
   */
  FunctionInterface(FunctionWarehouse & func_warehouse, InputParameters & params);

  Function & getFunction(std::string name);

private:
  //prefixed all member data with _func to prevent future Multiple Inheritance
  //issues. The compiler will complain even though it's private data
  InputParameters _func_params;
  FunctionWarehouse & _func_warehouse;
};

#endif //FUNCTIONINTERFACE_H
