//Moose includes
#include "MooseSystem.h"
#include "DofData.h"
#include "ComputeQPSolution.h"

//libmesh includes
#include "quadrature_gauss.h"
#include "dof_map.h"
#include "fe_base.h"

DofData::DofData(MooseSystem & moose_system) :
  _moose_system(moose_system),
  _current_elem(NULL)
{

}

DofData::~DofData()
{

}
