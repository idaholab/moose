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

#ifndef NODALNORMALSPREPROCESSOR_H
#define NODALNORMALSPREPROCESSOR_H

#include "ElementUserObject.h"
#include "SideUserObject.h"
#include "BoundaryRestrictable.h"
#include "libmesh/fe.h"

class NodalNormalsPreprocessor;
class AuxiliarySystem;

template<>
InputParameters validParams<NodalNormalsPreprocessor>();

/**
 *
 */
class NodalNormalsPreprocessor :
  public ElementUserObject,
  public BoundaryRestrictable
{
public:
  NodalNormalsPreprocessor(const std::string & name, InputParameters parameters);
  virtual ~NodalNormalsPreprocessor();

  virtual void initialize();
  virtual void finalize();
  virtual void execute();
  virtual void threadJoin(const UserObject & uo);

protected:
  AuxiliarySystem & _aux;
  FEType _fe_type;
  bool _has_corners;
  BoundaryID _corner_boundary_id;

  const VariablePhiGradient & _grad_phi;
};


#endif /* NODALNORMALSPREPROCESSOR_H */
