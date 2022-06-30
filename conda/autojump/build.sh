#!/bin/bash
set -eu

./install.py -f -d ${PREFIX} -p ${PREFIX}

mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export AUTOJUMP_SHELL=\`ps -o comm= \$\$ | sed -e 's/-//'\`
if [ -f ${PREFIX}/share/autojump/autojump.\${AUTOJUMP_SHELL} ]; then
  source ${PREFIX}/share/autojump/autojump.\${AUTOJUMP_SHELL}
fi
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset -f j autojump_chpwd jco jo jc 2>/dev/null
unset AUTOJUMP_SOURCED
EOF
