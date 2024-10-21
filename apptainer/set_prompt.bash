#!/bin/bash

PROMPT_COMMAND='if [[ $(type -t set_prompt) != function ]]; then source /.singularity.d/set_prompt.bash; fi; set_prompt'

set_prompt () {
    if [ -n "${CUSTOM_PROMPT}" ]; then
        PS1="${CUSTOM_PROMPT}"
        return
    fi

    local BOLD_RED='\[\e[01;31m\]'
    local GREEN='\[\e[00;32m\]'
    local PURPLE='\[\e[00;35m\]'
    local RED='\[\e[00;31m\]'
    local TEAL='\[\e[00;36m\]'
    local WHITE='\[\e[00;37m\]'
    local YELLOW='\[\e[00;33m\]'
    local RESET='\[\e[00m\]'

    PS1=""

    # Add the container version from ApptainerGenerator if it exists
    if [ -n "${MOOSE_APPTAINER_GENERATOR_NAME_SUMMARY}" ]; then
        PS1+="${YELLOW}${MOOSE_APPTAINER_GENERATOR_NAME_SUMMARY} "
    # Otherwise add the general container version if it exists
    elif [ -n "${SINGULARITY_NAME}" ]; then
        PS1+="${YELLOW}${SINGULARITY_NAME##*/} "
    fi

    # Begin prompt with a square bracket
    PS1+="${WHITE}["

    # Username in yellow
    PS1+="${YELLOW}\\u${WHITE}@"

    # Hostname in green
    PS1+="${GREEN}\\h${WHITE}: "

    # Add the git branch you are working on if in a git repo
    local BRANCH="$(git branch 2>/dev/null | sed -n 's/* \(.*\)/\1/p')"
    if [[ ! -z "${BRANCH}" ]]; then
        PS1+="${PURPLE}(${BRANCH}) "
    fi

    # Print the working directory in teal
    PS1+="${TEAL}\\w "

    # End prompt with a square bracket
    PS1+="${WHITE}]\n"

    # Print the prompt marker in green
    # and reset the text color to the default.
    PS1+="${GREEN}\\\$${RESET} "
}
