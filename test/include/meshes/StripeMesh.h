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

#ifndef STRIPEMESH_H_
#define STRIPEMESH_H_

#include "GeneratedMesh.h"

class StripeMesh;

template<>
InputParameters validParams<StripeMesh>();

/**
 * Mesh with subdomains as stripes
 *
 * NOTE: Tailored for rectangular meshes with quad elements
 */
class StripeMesh : public GeneratedMesh
{
public:
  StripeMesh(const std::string & name, InputParameters parameters);
  StripeMesh(const StripeMesh & other_mesh);
  virtual ~StripeMesh();

  virtual MooseMesh & clone() const;

  virtual void buildMesh();

protected:
  unsigned int _n_stripes;                      ///< number of stripes
};


#endif /* STRIPEMESH_H_ */
