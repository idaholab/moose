#!/usr/bin/env bash
APPLICATION_DIR=${PWD}

# Function for coloring progress messages
function message()
{
    local NO_COLOR="\033[0m"
    if [[ $2 == "green" ]]; then
        local COLOR="\033[32m"
    elif [[ $2 == "red" ]]; then
        local COLOR="\033[31m"
    elif [[ $2 == "yellow" ]]; then
        local COLOR="\033[33m"
    else
        local COLOR=${NO_COLOR}
    fi
    echo -e ${COLOR}$1${NO_COLOR}
}

# A function for updating a specific submodule in a given directory
# $1 = The directory to operate in
# $2 = The name of the submodule to update
function update_submodule()
{
    cd $1
    git_dir=`git rev-parse --show-cdup 2>/dev/null`
    if [[ $? == 0 && "x$git_dir" == "x" ]]; then
        git submodule init
        git submodule update -- $2

        if [[ $? != 0 ]]; then
            message "ERROR: git submodule command failed, are your proxy settings correct?" "red"
            exit 1
        fi
        message "  Sucessfully updated ${2} submodule" "green"

    else
        messeage "ERROR: Failed to call 'git', are you sure it is installed." "red"
        exit 1
    fi
}

# A function for updating MOOSE
function update_moose()
{
    message "Updating MOOSE"

    # If MOOSE_DIR is set
    if [ ! -z "$MOOSE_DIR" ]; then

        # Incidate that a user-defined MOOSE_DIR is being used
        message "WARNING: MOOSE_DIR set - overriding default installed path" "yellow"
        cd ${MOOSE_DIR}

        # Determine the current git branch for MOOSE
        BRANCH="$(git rev-parse --abbrev-ref HEAD)"
        if [[ "${BRANCH}" != "master" && "${BRANCH}" != "devel" ]]; then
            message "ERROR: You are the '${BRANCH}' branch within the MOOSE repository, only the 'devel' or 'master' branches support automatic updates" "red"
        else
            message "  Updating MOOSE repository..." "green"
            git pull --rebase git@github.com:idaholab/moose.git ${BRANCH}
        fi

    # Not using MOOSE_DIR, update the submodule within the application
    else
        export MOOSE_DIR=${APPLICATION_DIR}/moose
        update_submodule ${APPLICATION_DIR} "moose"
    fi

    message "  Using MOOSE at ${MOOSE_DIR}" "green"
    cd ${APPLICATION_DIR}
}

# A function for updating libMesh
function update_libmesh()
{
    message "Updating libMesh"

    # If LIBMESH_DIR is set
    if [ ! -z "$LIBMESH_DIR" ]; then

        # Indicate that the user-defined LIBMESH_DIR is being used
        message "WARNING: LIBMESH_DIR set - overriding default installed path" "yellow"
        message "         No cleaning will be done in specified path" "yellow"
        mkdir -p $LIBMESH_DIR

        cd ${LIBMESH_DIR}

        # Determine the current git branch for LIBMESH
        BRANCH="$(git rev-parse --abbrev-ref HEAD)"
        if [[ "${BRANCH}" != "master" ]]; then
            message "ERROR: You are the '${BRANCH}' branch within the libMesh repository, only the 'master' branch supports automatic updates" "red"
            exit 1
        else
            message "  Updating libMesh repository..." "green"
            git pull --rebase git@github.com:libMesh/libmesh.git ${BRANCH}
        fi

    # If not using LIBMESH_DIR, update the submodule
    else
        export LIBMESH_DIR=${MOOSE_DIR}/libmesh/installed
        if [ ! -d "${LIBMESH_DIR}" ]; then
            cd ${MOOSE_DIR}
            update_submodule ${MOOSE_DIR} "libmesh"
            mkdir ${LIBMESH_DIR}

        elif [ -d "${LIBMESH_DIR}" ]; then
            cd ${MOOSE_DIR}/libmesh
            rm -rf installed
            cd - >/dev/null # Make this quiet
            update_submodule ${MOOSE_DIR} "libmesh"

        else
            message "  Failed to update libMesh!" "red"
            exit 1
        fi
    fi

    message "  Using libMesh at ${LIBMESH_DIR}" "green"
    cd ${APPLICATION_DIR}
}

# Configure and compile libMesh
function build_libmesh()
{
    # Set the build methods of libMesh
    export METHODS=${METHODS:="opt dbg"}

    # Check if the user has ccache configured
    DISABLE_TIMESTAMPS=""
    echo $CXX | cut -d ' ' -f1 | grep '^ccache$' > /dev/null
    if [ $? == 0 ]; then
        # check if timestamps are explicitly enabled
        echo "$* " | grep -- '--enable-timestamps ' > /dev/null
        if [ $? == 0 ]; then
            message "WARNING: setting --enable-timestamps explicitly will negatively impact the ccache performance" "yellow"
        else
            message "Configuring limbesh with --disable-timestamps to improve ccache hit rate"
            DISABLE_TIMESTAMPS="--disable-timestamps"
        fi
    fi

    # Configure and build libMesh
    cd ${MOOSE_DIR}/libmesh
    rm -rf build
    mkdir build
    cd build
    ../configure --with-methods="${METHODS}" \
                 --prefix=$LIBMESH_DIR \
                 --enable-silent-rules \
                 --enable-unique-id \
                 --disable-warnings \
                 --disable-cxx11 \
                 --enable-unique-ptr \
                 --enable-openmp \
                 $DISABLE_TIMESTAMPS $*

    # let LIBMESH_JOBS be either MOOSE_JOBS, or 1 if MOOSE_JOBS
    # is not set (not using our package). Make will then build
    # with either JOBS if set, or LIBMESH_JOBS.
    message "Building and installing libMesh..." "green"
    LIBMESH_JOBS=${MOOSE_JOBS:-1}
    if [ -z "${MOOSE_MAKE}" ]; then
        make -j ${JOBS:-$LIBMESH_JOBS} && make install
    else
        ${MOOSE_MAKE} && ${MOOSE_MAKE} install
    fi
    message "libMesh build and install complete" "green"
}

# Compile MOOSE
function build_moose()
{
    # Build MOOSE
    message "Building the MOOSE framework..." "green"
    cd ${MOOSE_DIR}/framework
    if [ -z "${MOOSE_MAKE}" ]; then
        make -j ${JOBS:-$LIBMESH_JOBS}
    else
        ${MOOSE_MAKE}
    fi
    message "MOOSE framework build complete" "green"

}

############
### MAIN ###
############
update_moose
update_libmesh
build_libmesh
build_moose
message "Update complete" "green"
