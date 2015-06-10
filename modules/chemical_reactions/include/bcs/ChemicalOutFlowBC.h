/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHEMICALOUTFLOWBC_H
#define CHEMICALOUTFLOWBC_H

#include "IntegratedBC.h"

//libMesh includes
//#include "vector_value.h"


//Forward Declarations
class ChemicalOutFlowBC;

template<>
InputParameters validParams<ChemicalOutFlowBC>();

/**
 * Implements a simple constant VectorNeumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class ChemicalOutFlowBC : public IntegratedBC
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  ChemicalOutFlowBC(const std::string & name, InputParameters parameters);

 virtual ~ChemicalOutFlowBC(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  /**
   * Vector to dot with the normal.
   */
  const MaterialProperty<Real> & _diff;
  const MaterialProperty<Real> & _porosity;

//  std::vector<RealGradient> & _grad_p;

};

#endif //NEUMANNBC_H
