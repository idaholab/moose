//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StripeMesh.h"

#include "libmesh/elem.h"

registerMooseObject("MooseTestApp", StripeMesh);

InputParameters
StripeMesh::validParams()
{
  InputParameters params = GeneratedMesh::validParams();

  params.addRequiredParam<unsigned int>("stripes", "Number of stripes in the mesh");

  return params;
}

StripeMesh::StripeMesh(const InputParameters & parameters)
  : GeneratedMesh(parameters), _n_stripes(getParam<unsigned int>("stripes"))
{
  // The StripeMesh class only works with ReplicatedMesh
  errorIfDistributedMesh("StripeMesh");
}

StripeMesh::StripeMesh(const StripeMesh & other_mesh)
  : GeneratedMesh(other_mesh), _n_stripes(other_mesh._n_stripes)
{
}

StripeMesh::~StripeMesh() {}

MooseMesh &
StripeMesh::clone() const
{
  return *(new StripeMesh(*this));
}

void
StripeMesh::buildMesh()
{
  GeneratedMesh::buildMesh();

  Real h = (getParam<Real>("xmax") - getParam<Real>("xmin")) / _n_stripes; // width of the stripe

  for (unsigned int en = 0; en < nElem(); en++)
  {
    // get an element
    Elem * e = elemPtr(en);

    if (!e)
    {
      mooseError("Error getting element ", en, ". StripeMesh only works with ReplicatedMesh...");
    }
    else
    {
      Point centroid = e->vertex_average(); // get its centroid
      subdomain_id_type sid =
          floor((centroid(0) - getParam<Real>("xmin")) / h); // figure out the subdomain ID
      e->subdomain_id() = sid;
    }
  }
}
