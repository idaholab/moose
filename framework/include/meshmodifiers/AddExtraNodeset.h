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

#ifndef ADDEXTRANODESET_H
#define ADDEXTRANODESET_H

#include "MeshModifier.h"

//Forward Declaration
class MooseMesh;

class AddExtraNodeset;

template<>
InputParameters validParams<AddExtraNodeset>();

class AddExtraNodeset : public MeshModifier
{
public:
  AddExtraNodeset(const std::string & name, InputParameters params);

  virtual void modify();
};

#endif // ADDEXTRANODESET_H
