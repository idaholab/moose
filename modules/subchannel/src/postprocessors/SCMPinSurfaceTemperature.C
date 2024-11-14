/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "SCMPinSurfaceTemperature.h"
#include "SolutionHandle.h"
#include "FEProblemBase.h"
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "libmesh/system.h"

registerMooseObject("SubChannelApp", SCMPinSurfaceTemperature);

InputParameters
SCMPinSurfaceTemperature::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Returns the surface temperature of a specific fuel pin at a user defined height");
  params.addRequiredParam<Real>("height", "Axial location on fuel pin [m]");
  params.addRequiredParam<int>("index", "Index of fuel pin");
  return params;
}

SCMPinSurfaceTemperature::SCMPinSurfaceTemperature(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(libMesh::cast_ref<SubChannelMesh &>(_fe_problem.mesh())),
    _height(getParam<Real>("height")),
    _i_pin(getParam<int>("index")),
    _value(0)
{
  if (!_mesh.pinMeshExist())
    mooseError(
        name(),
        " : The SCMPinSurfaceTemperature post processor calculates temperature on pins. A Pin "
        "Mesh should be defined.");
}

void
SCMPinSurfaceTemperature::execute()
{
  auto Tpin_soln = SolutionHandle(_fe_problem.getVariable(0, "Tpin"));
  auto nz = _mesh.getNumOfAxialCells();
  auto z_grid = _mesh.getZGrid();
  auto total_length =
      _mesh.getHeatedLength() + _mesh.getHeatedLengthEntry() + _mesh.getHeatedLengthExit();

  if (_height >= total_length)
  {
    auto * node = _mesh.getPinNode(_i_pin, nz);
    _value = Tpin_soln(node);
  }
  else
  {
    for (unsigned int iz = 0; iz < nz; iz++)
    {
      if (_height >= z_grid[iz] && _height < z_grid[iz + 1])
      {
        auto * node_out = _mesh.getPinNode(_i_pin, iz + 1);
        auto * node_in = _mesh.getPinNode(_i_pin, iz);
        _value = Tpin_soln(node_in) + (Tpin_soln(node_out) - Tpin_soln(node_in)) *
                                          (_height - z_grid[iz]) / (z_grid[iz + 1] - z_grid[iz]);
        break;
      }
    }
  }
}

Real
SCMPinSurfaceTemperature::getValue() const
{
  return _value;
}
