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

#include "StripeMesh.h"

template<>
InputParameters validParams<StripeMesh>()
{
  InputParameters params = validParams<GeneratedMesh>();

  params.addRequiredParam<unsigned int>("stripes", "Number of stripes in the mesh");

  return params;
}

StripeMesh::StripeMesh(const std::string & name, InputParameters parameters) :
    GeneratedMesh(name, parameters),
    _n_stripes(getParam<unsigned int>("stripes"))
{
  // The StripeMesh class only works with SerialMesh
  errorIfParallelDistribution("StripeMesh");
}

StripeMesh::StripeMesh(const StripeMesh & other_mesh) :
    GeneratedMesh(other_mesh),
    _n_stripes(other_mesh._n_stripes)
{
}

StripeMesh::~StripeMesh()
{
}

MooseMesh &
StripeMesh::clone() const
{
  return *(new StripeMesh(*this));
}

void
StripeMesh::buildMesh()
{
  GeneratedMesh::buildMesh();

  Real h = (getParam<Real>("xmax") - getParam<Real>("xmin")) / _n_stripes;  // width of the stripe

  for (unsigned int en = 0; en < nElem(); en++)
  {
    // get an element
    Elem * e = elem(en);

    if (!e)
    {
      mooseError("Error getting element " << en << ". StripeMesh only works with SerialMesh...");
    }
    else
    {
    Point centroid = e->centroid();                             // get its centroid
    subdomain_id_type sid = floor((centroid(0) - getParam<Real>("xmin")) / h);   // figure out the subdomain ID
    e->subdomain_id() = sid;
    }
  }
}

