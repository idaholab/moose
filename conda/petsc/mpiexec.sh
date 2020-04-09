#!/bin/bash
set -euo pipefail

if command -v "${PREFIX}/bin/mpichversion" >/dev/null; then
    export HYDRA_LAUNCHER=fork
    mpiexec="mpiexec"
fi

if command -v "${PREFIX}/bin/ompi_info" >/dev/null; then
    export OMPI_MCA_plm=isolated
    export OMPI_MCA_rmaps_base_oversubscribe=yes
    export OMPI_MCA_btl_vader_single_copy_mechanism=none
    mpiexec="mpiexec --allow-run-as-root"
fi

# pipe stdout, stderr through cat to avoid O_NONBLOCK issues
$mpiexec $@ 2>&1 | cat
