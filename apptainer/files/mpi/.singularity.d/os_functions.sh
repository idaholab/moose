#!/bin/sh

if ! which grep; then
    echo "grep is not available"
    exit 1
fi

is_rocky() {
    if ! grep -q "NAME=\"Rocky" /etc/os-release; then
        return 1
    fi
}

is_rocky8() {
    if is_rocky && ! grep -q 'VERSION=\"8.' /etc/os-release; then
        return 1
    fi
}

is_rocky9() {
    if is_rocky && ! grep -q 'VERSION=\"9.' /etc/os-release; then
        return 1
    fi
}

is_ubuntu() {
    if ! grep -q "NAME=\"Ubuntu" /etc/os-release; then
        return 1
    fi
}

update_system_packages() {
    if is_ubuntu; then
        apt-get update
    fi
}

clean_system_packages() {
    if is_rocky; then
        dnf clean all
    elif is_ubuntu; then
        apt-get autoremove -y
        apt-get clean
        rm -rf /var/lib/apt/lists/*
    fi
}
