#include "Problem.h"

Problem::Problem()
{
  unsigned int n_threads = libMesh::n_threads();

  _real_zero.resize(n_threads);
  _zero.resize(n_threads);
  _grad_zero.resize(n_threads);
  _second_zero.resize(n_threads);
}

Problem::~Problem()
{
}

