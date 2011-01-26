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

#ifndef CONTACTMASTER_H
#define CONTACTMASTER_H

// Moose Includes
#include "DiracKernel.h"
#include "PenetrationLocator.h"

//Forward Declarations
class ContactMaster;

template<>
InputParameters validParams<ContactMaster>();

class ContactMaster : public DiracKernel
{
public:
  ContactMaster(const std::string & name, InputParameters parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
protected:
  const unsigned int _component;
  PenetrationLocator & _penetration_locator;
  NumericVector<Number> & _residual_copy;
  SparseMatrix<Number> & _jacobian_copy;

  std::map<Point, PenetrationLocator::PenetrationInfo *> point_to_info;

  unsigned int _x_var;
  unsigned int _y_var;
  unsigned int _z_var;

  RealVectorValue _vars;
};
 
#endif //CONTACTMASTER_H
