/**********************************************************************/
/*                     DO NOT MODIFY THIS HEADER                      */
/* MAGPIE - Mesoscale Atomistic Glue Program for Integrated Execution */
/*                                                                    */
/*            Copyright 2017 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

#include "ThreadedRadialGreensConvolutionLoop.h"
#include "Function.h"

ThreadedRadialGreensConvolutionLoop::ThreadedRadialGreensConvolutionLoop(
    RadialGreensConvolution & green)
  : _green(green), _function_name(_green.parameters().get<FunctionName>("function"))
{
}

// Splitting Constructor
ThreadedRadialGreensConvolutionLoop::ThreadedRadialGreensConvolutionLoop(
    const ThreadedRadialGreensConvolutionLoop & x, Threads::split /*split*/)
  : _green(x._green), _function_name(x._function_name)
{
}

void
ThreadedRadialGreensConvolutionLoop::operator()(const QPDataRange & qpdata_range)
{
  // fetch data from parent
  const auto r_cut = _green._r_cut;
  const auto & qp_data = _green._qp_data;
  const auto & correction_integral = _green._correction_integral;
  const auto & kd_tree = _green._kd_tree;
  const auto dim = _green._dim;
  const auto t = _green._t;

  // fetch thread copy of the function for safe evaluation
  ParallelUniqueId puid;
  const Function & function = _green._fe_problem.getFunction(_function_name, puid.id);

  // radial bin size
  const Real dr = r_cut / correction_integral.size();

  // integral of the convolution (used if normalization is requested)
  _convolution_integral = 0.0;

  // tree search data structures
  std::vector<std::pair<std::size_t, Real>> ret_matches;
  nanoflann::SearchParams search_params;

  // result map entry
  const auto end_it = _green._convolution.end();
  auto it = end_it;

  // iterate over qp range
  for (auto && local_qp : qpdata_range)
  {
    // Look up result map iterator only if we enter a new element. this saves a bunch
    // of map lookups because same element entries are consecutive in the qp_data vector.
    if (it == end_it || it->first != local_qp._elem_id)
      it = _green._convolution.find(local_qp._elem_id);

    // initialize result entry
    mooseAssert(it != end_it, "Current element id not found in result set.");
    auto & sum = it->second[local_qp._qp];
    sum = 0.0;

    // if the variable is periodic we need to perform extra searches translated onto
    // the periodic neighbors
    std::list<Point> cell_vector = {Point()};
    for (unsigned int j = 0; j < dim; ++j)
      if (_green._periodic[j])
      {
        std::list<Point> new_cell_vector;

        for (const auto & cell : cell_vector)
        {
          if (local_qp._q_point(j) + _green._periodic_vector[j](j) - r_cut <
              _green._periodic_max[j])
            new_cell_vector.push_back(cell + _green._periodic_vector[j]);

          if (local_qp._q_point(j) - _green._periodic_vector[j](j) + r_cut >
              _green._periodic_min[j])
            new_cell_vector.push_back(cell - _green._periodic_vector[j]);
        }

        cell_vector.splice(cell_vector.end(), new_cell_vector);
      }

    // perform radius search and aggregate data considering potential periodicity
    Point center;
    for (const auto & cell : cell_vector)
    {
      ret_matches.clear();
      center = local_qp._q_point + cell;
      std::size_t n_result =
          kd_tree->radiusSearch(&(center(0)), r_cut * r_cut, ret_matches, search_params);
      for (std::size_t j = 0; j < n_result; ++j)
      {
        const auto & other_qp = qp_data[ret_matches[j].first];
        const Real r = std::sqrt(ret_matches[j].second);

        // R is the equivalent sphere radius for the quadrature point. The spherical integral
        // integral_0^R 1/(4pi*r^2) *4pi*r^2 = R
        // So R is the integral over the geometric attenuation at the center quadrature point
        switch (dim)
        {
          case 1:
          {
            // correction integral is the integral over the geometric attenuation
            // times the Green's function over a 2D disc inscribed into the r_cut
            // sphere and perpendicular to the mesh.
            Real add = correction_integral[std::floor(r / dr)] * other_qp._volume;

            if (r == 0)
            {
              const Real R = 0.5 * other_qp._volume;
              add += function.value(t, Point(0.0, 0.0, 0.0)) *
                     (
                         // add the center sphere attenuation integral
                         R +
                         // add the section missing or overlapping between correction_integral and
                         // center sphere
                         _green.attenuationIntegral(R, _green._zero_dh, 0.0, 1) * other_qp._volume);
            }
            sum += add * other_qp._value;
            break;
          }

          case 2:
          {
            // correction integral is the integral over the geometric attenuation
            // times the Green's function over a 1D line segment inscribed into the r_cut
            // sphere and perpendicular to the mesh.
            Real add = correction_integral[std::floor(r / dr)] * other_qp._volume;
            if (r == 0)
            {
              const Real R = std::sqrt(other_qp._volume / (2.0 * libMesh::pi));
              add += function.value(t, Point(0.0, 0.0, 0.0)) *
                     (
                         // add the center sphere attenuation integral
                         R +
                         // add the section missing or overlapping between correction_integral and
                         // center sphere
                         _green.attenuationIntegral(R, _green._zero_dh, 0.0, 2) * other_qp._volume);
            }
            sum += add * other_qp._volume * other_qp._value;
            break;
          }

          case 3:
          {
            const Real G = function.value(t, Point(r, 0.0, 0.0));
            if (r == 0)
            {
              // R is the integral over the geometric attenuation in a sphere around the origin
              const Real R = std::cbrt(3.0 / 4.0 * other_qp._volume / libMesh::pi);
              sum += G * R * other_qp._value;
            }
            else
              sum += G * 0.25 / (libMesh::pi * r * r) * other_qp._volume * other_qp._value;
          }
        }
      }
    }

    // integrate the convolution result
    _convolution_integral += sum;
  }
}
