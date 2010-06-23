#ifndef AUXKERNEL_H
#define AUXKERNEL_H

#include "Moose.h"
#include "MooseArray.h"
#include "PDEBase.h"
#include "MaterialPropertyInterface.h"

//forward declarations
class AuxKernel;
class MooseSystem;
class ElementData;
class AuxData;

template<>
InputParameters validParams<AuxKernel>();

/** 
 * AuxKernels compute values at nodes.
 */
class AuxKernel :
  public PDEBase,
  protected MaterialPropertyInterface
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  AuxKernel(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~AuxKernel(){}

  void computeAndStore(THREAD_ID tid);

  bool isNodal();

protected:
  virtual Real computeValue() = 0;

  virtual Real computeQpResidual();
  
  /**
   * Convenience reference to the ElementData object inside of MooseSystem
   */
  ElementData & _element_data;

  /**
   * Convenience reference to the AuxData object inside of MooseSystem
   */
  AuxData & _aux_data;

  /**
   * Current material
   */
  Material * & _material;

  bool _nodal;

  Real & _u_aux;  
  Real & _u_old_aux;
  Real & _u_older_aux;

  Real & coupledValAux(std::string name, int i = 0);
  Real & coupledValOldAux(std::string name, int i = 0);
  Real & coupledValOlderAux(std::string name, int i = 0);

  RealGradient & coupledGradAux(std::string name, int i = 0);
  RealGradient & coupledGradOldAux(std::string name, int i = 0);
  RealGradient & coupledGradOlderAux(std::string name, int i = 0);


  /*************
   * Nodal Stuff
   *************/
  /**
   * Current Node
   */
  const Node * & _current_node;
};

#endif //AUXKERNEL_H
