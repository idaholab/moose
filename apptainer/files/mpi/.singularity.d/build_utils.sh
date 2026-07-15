#!/bin/sh

if ! which grep; then
    echo "grep is not available"
    exit 1
fi

# Whether or not the operating system is rocky-based,
is_rocky() {
    if ! grep -q "NAME=\"Rocky" /etc/os-release; then
        return 1
    fi
}

# Whether or not the operating system is rocky 8,
is_rocky8() {
    if is_rocky && ! grep -q 'VERSION=\"8.' /etc/os-release; then
        return 1
    fi
}

# Whether or not the operating system is rocky 9,
is_rocky9() {
    if is_rocky && ! grep -q 'VERSION=\"9.' /etc/os-release; then
        return 1
    fi
}

# Whether or not the operating system is ubuntu,
is_ubuntu() {
    if ! grep -q "NAME=\"Ubuntu" /etc/os-release; then
        return 1
    fi
}

# Update the system packages. Useful at the beginning of
# a build, in which ubuntu requires an update before installing,
update_system_packages() {
    if is_ubuntu; then
        apt-get update
    fi
}

# Cleanup the system packages. Useful at the end of a build,
# in which the downloaded package cache should be cleaned up.
clean_system_packages() {
    if is_rocky; then
        dnf clean all
    elif is_ubuntu; then
        apt-get autoremove -y
        apt-get clean
        rm -rf /var/lib/apt/lists/*
    fi
}

# Determine the default value for -march for a build. Needed
# because older compilers (GCC 9, our minimum) do not support
# things like "x86-64-v3".
default_march() {
    if ! command -v mpicxx >/dev/null 2>&1; then
        echo "mpicxx not found" >&2
        return 1
    fi

    COMPILER="$(mpicxx -show | awk '{print $1}')"

    if "$COMPILER" --version | grep -qi clang; then
        echo "x86-64-v3"
    elif "$COMPILER" --version | grep -qi "gcc\|g++"; then
        GCC_MAJOR=$("$COMPILER" -dumpfullversion -dumpversion | cut -d. -f1)
        if [ "$GCC_MAJOR" -ge 10 ]; then
            echo "x86-64-v3"
        else
            echo "haswell"
        fi
    else
        echo "unknown compiler" >&2
        return 1
    fi
}
