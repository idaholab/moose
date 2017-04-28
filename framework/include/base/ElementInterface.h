/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ELEMENTINTERFACE_H
#define ELEMENTINTERFACE_H

#include "Moose.h"

// Forward declarations
class ElementInterface;
class FEProblemBase;
class InputParameters;
class MooseObject;
class SubProblem;
class Assembly;

#include "MooseMesh.h"
#include "MooseVariable.h"
#include "libmesh/dense_vector.h"
#include "libmesh/dense_matrix.h"

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<ElementInterface>();

class ElementInterface
{
public:
  ElementInterface(const MooseObject * moose_object);
  virtual ~ElementInterface();

  void massProduct(const VariableValue & u, Real coef, DenseVector<Real> & prod) const;
  void stiffnessProduct(const VariableValue & u, Real coef, DenseVector<Real> & prod) const;

  void massMatrix(Real coef, DenseMatrix<Real> & mat) const;
  void stiffnessMatrix(Real coef, DenseMatrix<Real> & mat) const;

protected:
  const InputParameters & _ei_params;

  SubProblem & _ei_subproblem;
  FEProblemBase & _ei_feproblem;

  MooseMesh & _ei_mesh;
  Assembly & _ei_assembly;
  const Elem *& _ei_current_elem;
};

#endif /* ELEMENTINTERFACE_H */
