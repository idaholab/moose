#pragma once
#include "auxsolver_base.hpp"

namespace hephaestus
{

class LineSampler
{
public:
  LineSampler() = default;
  LineSampler(const mfem::ParGridFunction & gridfunction,
              mfem::Vector start_pos,
              mfem::Vector end_pos,
              unsigned int num_pts);

  ~LineSampler() { _finder.FreeData(); }

  void Solve(double t = 0.0)
  {
    _t = t;
    _finder.Interpolate(_vxyz, _gf, _interp_vals, _point_ordering);
  }

  void WriteToFile(std::ofstream & filestream, std::string sep = ", ");

private:
  const mfem::ParGridFunction & _gf;
  mfem::ParMesh & _pmesh;
  int _dim;
  int _vec_dim;
  int _num_pts;
  double _t{0.0};
  mfem::Ordering::Type _gf_ordering;
  mfem::Ordering::Type _point_ordering;
  mfem::Vector _vxyz;
  mfem::Vector _interp_vals;
  mfem::FindPointsGSLIB _finder;
};

// Wrapper for LineSampler to resample points every timestep and write to a file
class LineSamplerAux : public AuxSolver
{
public:
  LineSamplerAux(std::string gridfunction_name,
                 mfem::Vector start_pos,
                 mfem::Vector end_pos,
                 unsigned int num_pts,
                 std::string filename,
                 std::string header)
    : _gridfunction_name{std::move(gridfunction_name)},
      _start_pos{std::move(start_pos)},
      _end_pos{std::move(end_pos)},
      _num_pts(num_pts),
      _filename{std::move(filename)},
      _header{std::move(header)} {};

  void Init(const hephaestus::GridFunctions & gridfunctions,
            hephaestus::Coefficients & coefficients) override;

  void Solve(double t = 0.0) override;

  const LineSampler & GetLineSampler() { return *_line_sampler; }

private:
  std::string _gridfunction_name;
  std::shared_ptr<LineSampler> _line_sampler{nullptr};

  mfem::ParGridFunction * _gf{nullptr};
  mfem::Vector _start_pos;
  mfem::Vector _end_pos;
  unsigned int _num_pts;
  std::string _filename;
  std::string _header;
  std::ofstream _filestream;
};

} // namespace hephaestus
