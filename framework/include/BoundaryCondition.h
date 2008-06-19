#include "Kernel.h"

#ifndef BOUNDARYCONDITION_H
#define BOUNDARYCONDITION_H

/** 
 * Extrapolation of a Kernel for BC usage.  Children of this class should override
 * computeQpResidual() for use by computeSideResidual.
 */
class BoundaryCondition : public Kernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  BoundaryCondition(Parameters parameters, EquationSystems * es, std::string var_name, bool integrated, unsigned int boundary_id)
    :Kernel(parameters, es,var_name, integrated),
     _boundary_id(boundary_id)
  {}

  /**
   * Standalone constructor.
   */
  BoundaryCondition(EquationSystems * es, std::string var_name, bool integrated, unsigned int boundary_id)
    :Kernel(es,var_name, integrated),
     _boundary_id(boundary_id)
  {}

  virtual ~BoundaryCondition(){}

  /** 
   * Boundary ID the BC is active on.
   * 
   * @return The boundary ID.
   */
  unsigned int boundaryID(){ return _boundary_id; }

protected:

  /**
   * Boundary ID this BC is active on.
   */
  unsigned int _boundary_id;
};

#endif //BOUNDARYCONDITION_H
