# Sampler System

[samplers/Sampler.md] objects in [MOOSE] are designed to generate an arbitrary set of data sampled from
any number of Distribution objects.

The sampler operators by returning a vector (`std::vector<Real>`) or matrix
(`libMesh::DenseMatrix<Real>`) from one of three methods:

- +`getNextLocalRow`+\\
  This method returns a single row from the complete sample matrix. +This is the preferred method for
  accessing sample data, since the memory footprint is limited to a single row+ rather than
  potentially large matrices as in the other methods. This method should be used as follows:

  ```c++
  for (dof_id_type i = getLocalRowBegin(); i < getLocalRowEnd(); ++i)
      std::vector<Real> row = getNextLocalRow();
  ```

- +`getLocalSamples`+\\
  This method returns a subset of rows from the sample matrix for the current processor. This matrix
  is populated on demand and should not be stored to ensure a low memory footprint. This is the
  preferred method for accessing sample data.

- +`getSamples`+\\
  This method returns the complete sample matrix from the Sampler object, the matrix is populated
  on-demand and should not be stored since it often very large and thus can consume a significant
  amount of memory. Generally, this method should be avoid in favor of `getLocalSamples`.

The system is designed such that each row in the matrix represents a complete set of samples that is
passed to sub-applications via the [SamplerTransientMultiApp.md] or [SamplerFullSolveMultiApp.md].
