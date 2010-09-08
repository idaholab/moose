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

#ifndef MESHBLOCK_H
#define MESHBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericVariableBlock;
class MeshBlock;

namespace libMesh
{
  class MeshRefinement;
  class Mesh;
}

template<>
InputParameters validParams<MeshBlock>();

class MeshBlock: public ParserBlock
{
public:
  MeshBlock(const std::string & name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  bool autoResizeProblem(Mesh *mesh, MeshRefinement &mesh_refinement);
  bool checkVariableProperties(bool (GenericVariableBlock::*property)() const);
};

  

#endif //MESHBLOCK_H
