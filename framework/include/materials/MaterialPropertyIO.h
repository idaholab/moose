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

#ifndef MATERIALPROPERTYIO_H
#define MATERIALPROPERTYIO_H

#include "Moose.h"
#include "ColumnMajorMatrix.h"
#include "DataIO.h"

//libMesh
#include "libmesh/dense_matrix.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class MooseMesh;
class FEProblem;
class MaterialPropertyStorage;

/**
 * This class saves stateful material properties into a file.
 *
 */
class MaterialPropertyIO
{
public:
  MaterialPropertyIO(FEProblem & fe_problem);
  virtual ~MaterialPropertyIO();

  virtual void write(const std::string & file_name);
  virtual void read(const std::string & file_name);

protected:
  FEProblem & _fe_problem;
  MooseMesh & _mesh;
  MaterialPropertyStorage & _material_props;
  MaterialPropertyStorage & _bnd_material_props;

  static const unsigned int file_version;
};

#endif /* MATERIALPROPERTYIO_H */
