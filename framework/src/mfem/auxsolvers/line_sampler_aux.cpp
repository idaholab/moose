#include "line_sampler_aux.hpp"

namespace hephaestus
{
LineSampler::LineSampler(const mfem::ParGridFunction & gridfunction,
                         mfem::Vector start_pos,
                         mfem::Vector end_pos,
                         unsigned int num_pts)
  : _gf(gridfunction),
    _pmesh(*gridfunction.ParFESpace()->GetParMesh()),
    _dim(_pmesh.Dimension()),
    _vec_dim(gridfunction.VectorDim()),
    _num_pts(num_pts),
    _gf_ordering(gridfunction.ParFESpace()->GetOrdering()),
    _point_ordering(_gf_ordering),
    _vxyz(num_pts * _dim),
    _interp_vals(num_pts * _vec_dim),
    _finder(_pmesh.GetComm())
{
  // Use a dummy SegmentElement to create sample points along line
  mfem::L2_SegmentElement el(num_pts - 1, mfem::BasisType::ClosedUniform);
  const mfem::IntegrationRule & ir = el.GetNodes();
  for (int i = 0; i < ir.GetNPoints(); i++)
  {
    const mfem::IntegrationPoint & ip = ir.IntPoint(i);
    if (_point_ordering == mfem::Ordering::byNODES)
    {
      _vxyz(i) = start_pos(0) + ip.x * (end_pos(0) - start_pos(0));
      _vxyz(num_pts + i) = start_pos(1) + ip.x * (end_pos(1) - start_pos(1));
      _vxyz(2 * num_pts + i) = start_pos(2) + ip.x * (end_pos(2) - start_pos(2));
    }
    else
    {
      _vxyz(i * _dim + 0) = start_pos(0) + ip.x * (end_pos(0) - start_pos(0));
      _vxyz(i * _dim + 1) = start_pos(1) + ip.x * (end_pos(1) - start_pos(1));
      _vxyz(i * _dim + 2) = start_pos(2) + ip.x * (end_pos(2) - start_pos(2));
    }
  }
  // Find and interpolate FE function values on the desired points.
  _finder.Setup(_pmesh);
}

void
LineSampler::WriteToFile(std::ofstream & filestream, std::string sep)
{
  // Print the results for task 0 since either 1) all tasks have the
  // same set of points or 2) only task 0 has any points.
  int myid;
  MPI_Comm_rank(_pmesh.GetComm(), &myid);
  if (myid == 0)
  {
    for (int i = 0; i < _num_pts; i++)
    {
      filestream << _t << sep;
      if (_point_ordering == mfem::Ordering::byNODES)
      {
        filestream << _vxyz(i) << sep << _vxyz(_num_pts + i) << sep << _vxyz(2 * _num_pts + i)
                   << sep;
      }
      else
      {
        filestream << _vxyz(i * _dim + 0) << sep << _vxyz(i * _dim + 1) << sep
                   << _vxyz(i * _dim + 2) << sep;
      }
      for (int j = 0; j < _vec_dim; j++)
      {
        filestream << (_gf_ordering == mfem::Ordering::byNODES ? _interp_vals[i + j * _num_pts]
                                                               : _interp_vals[i * _vec_dim + j])
                   << sep;
      }
      filestream << "\n";
    }
  }
}

void
LineSamplerAux::Init(const hephaestus::GridFunctions & gridfunctions,
                     hephaestus::Coefficients & coefficients)
{
  _gf = gridfunctions.Get(_gridfunction_name);
  _line_sampler = std::make_shared<LineSampler>(*_gf, _start_pos, _end_pos, _num_pts);

  _filestream.open(_filename);
  _filestream << _header << "\n";
  _filestream.close();
}

void
LineSamplerAux::Solve(double t)
{
  if (_line_sampler != nullptr)
  {
    _line_sampler->Solve(t);

    _filestream.open(_filename, std::ios_base::app);
    _line_sampler->WriteToFile(_filestream);
    _filestream.close();
  }
}

} // namespace hephaestus
