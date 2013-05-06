#!/bin/bash

#   verbose
set +o nounset # temporary do not check uninitialized variables
if ! [ -n "${SAFE_EXECUTE_VERBOSE}" ]
then
    export SAFE_EXECUTE_VERBOSE="0"
fi

#   options
set -o nounset  # avoid uninitialized variables
set -o errexit  # exit when statement returns false
set -o pipefail # pipe fail

##   do not set locale here, but outside
#export LC_ALL="C"
#export LANG="C"
#export LANGUAGE="C"

#   constant 
tab="	"
endl="\n"

#   Usage:
#       safe_let expr_str
#   Function:
#       The build-in command 'let' returns: 0, if the result of given expression is non-zero; 1, otherwise.
#       This wrapping function always return 0.
function safe_let
{
    if [ "$#" -lt 1 ]
    then
        echo "${FUNCNAME[0]}: expr_str" 1>&2
        exit 1
    fi
    local expr_str="${1}"
    if ! let "${expr_str}"
    then
        true   #   do nothing
    fi
    return 0
}

#   Usage:
#       safe_expr expr_str ...
#   Function:
#       The command 'expr' returns: 0, if the result of given expression is non-zero; 1, otherwise.
#       This wrapping function always return 0.
function safe_expr
{
    if [ "$#" -lt 1 ]
    then
        echo "${FUNCNAME[0]}: expr_str ..." 1>&2
        exit 1
    fi
    #   check
    local cmd="expr"
    local expr_str
    for expr_str in "$@"
    do
        cmd="${cmd} \"${expr_str}\""
    done
    set +o errexit
    eval "${cmd}"
    local err_code="$?"
    if [ "${err_code}" -eq 0 ] || [ "${err_code}" -eq 1 ]
    then
        true   #   do nothing, exit depending on result
    else
        #   TODO: print call stack
        echo "expr: err_code=${err_code}: cmd: ${cmd}"
        exit 1
    fi
    set -o errexit
    return 0
}

#   Usage:
#       safe_execute cmd_str
#   Function:
#       Safely execute given command string, if failed, exit the script with error.
function safe_execute
{
    if [ "$#" -lt 1 ]
    then
        echo "${0}: ${FUNCNAME[0]} cmd_str" 1>&2
        exit 1
    fi
    if [ -n "${SAFE_EXECUTE_VERBOSE}" ] && [ "${SAFE_EXECUTE_VERBOSE}" -ne 0 ]
    then
        echo "${FUNCNAME[0]}: \"$@\"" 1>&2
    fi
    set +o errexit
    eval "$@"
    local err_code="$?"
    if [ "${err_code}" -ne 0 ]
    then
        echo "${0}: failed, exit code is ${err_code}, call stack:" 1>&2
        local i
        local func_id=0
        for ((i=0; i<${#FUNCNAME[@]}; ++i))
        do
            local func_name="${FUNCNAME[${i}]}"
            local file_name="${BASH_SOURCE[${i}]}"
            local line_no
            if [ "${i}" -eq 0 ]
            then
                line_no="${LINENO}"
            else
                line_no="${BASH_LINENO[${i}-1]}" 1>&2
            fi
            if [ "${i}" -eq 0 ] || [ "${func_name}" != "${FUNCNAME[0]}" ]
            then
                echo "[${func_id}]: function \"${FUNCNAME[${i}]}\" called from file ${file_name}:${line_no}" 1>&2
                safe_let "func_id=${func_id}+1"
            fi
        done
        exit "${err_code}"
    fi
    set -o errexit
    return 0
}

#   Usage:
#       safe_mkdir dir_name
#   Function:
#       Make directory with all its parent directories.
#       If the given directory already exists, do nothing.
function safe_mkdir
{
    if [ "$#" -lt 1 ]
    then
        echo "${0}: ${FUNCNAME[0]} dir_name" 1>&2
        exit 1
    fi
    local dir_name="${1}"
    if [ -e "${dir_name}" ] && [ -d "${dir_name}" ]
    then
        return 0
    fi
    safe_execute "mkdir -p ${dir_name}"
    return 0
}

#   Usage:
#       safe_rm param [param]...
#   Function:
#       Remove given files and directories given in param.
#       If the given files or directories do not exist, do nothing, i.e. do NOT exit with error.
function safe_rm
{
    if [ "$#" -lt 1 ]
    then
        echo "${0}: ${FUNCNAME[0]} param [param]..." 1>&2
        exit 1
    fi
    #   check
    local f
    for f in "$@"
    do
        if [ -e "${f}" ] || [ -L "${f}" ]
        then
            safe_execute "rm -rf \"${f}\""
        fi
    done
    return 0
}

#   Usage:
#       safe_ln src_path tgt_path
#   Function:
#       Symbolically link file or directory.
#       If the target object already exists, first delete it, then link.
function safe_ln
{
    if [ "$#" -lt 2 ]
    then
        echo "${0}: ${FUNCNAME[0]} src_path tgt_path" 1>&2
        exit 1
    fi
    local src_path="${1}"
    local tgt_path="${2}"
    if ! [ -e "${src_path}" ]
    then
        echo "${0}: safe_ln failed: source file does not exist: ${src_path}" 1>&2
        exit 1
    fi
    safe_rm "${tgt_path}"
    safe_execute "ln -s ${src_path} ${tgt_path}"
    return 0
}

#   safe link files, but do relative link
#   if link already exist, remove it first, then do link
#   the inputs must be absolute path
function safe_rln
{
    if [ "$#" -lt 2 ]
    then
        echo "$#" 1>&2
        echo "${0}: ${FUNCNAME[0]} failed: no input file is given" 1>&2
        exit 1
    fi
    local src_path="${1}"
    local tgt_path="${2}"
    #   check
    if [ "${src_path:0:1}" != "/" ]
    then
        echo "$#" 1>&2
        echo "${0}: ${FUNCNAME[0]} failed: only support absolute path" 1>&2
        exit 1
    fi
    if [ "${tgt_path:0:1}" != "/" ]
    then
        echo "$#" 1>&2
        echo "${0}: ${FUNCNAME[0]} failed: only support absolute path" 1>&2
        exit 1
    fi
    #   get common prefix
    local rel_path=""
    safe_execute "get_rel_path ${tgt_path} ${src_path} rel_path"
    echo "src_path=${src_path}"
    echo "tgt_path=${tgt_path}"
    echo "rel_path=${rel_path}"
    #   minor correction for file
    if ! [ -d "${src_path}" ]
    then
        echo ""
    fi
    local common_prefix=$(printf "%s\n%s\n" "${src_path}" "${tgt_path}" | sed -e 'N;s/^\(.*\).*\n\1.*$/\1/')
    src_path="${src_path#${common_prefix}}"
    tgt_path="${tgt_path#${common_prefix}}"
    echo "not implement yet" 1>&2
    return
    exit 11
    safe_ln "${src_path}" "${tgt_path}"
    return 0
}

#   Usage:
#       equal_line file [file]...
#   Function:
#       If input files are with equal lines, return 0, else return non-zero value.
function equal_line
{
    #   check no input file
    if [ "$#" -lt 1 ]
    then
        echo "${0}: ${FUNCNAME[0]} file [file]..." 1>&2
        return 1
    fi
    #   first file
    local first_file="${1}"
    #   first line num
    local first_num=$(wc -l < "${first_file}")
    if [ $? != 0 ]
    then
        echo "${0}: ${FUNCNAME[0]} failed: \"wc -l ${first_file}\"" 1>&2
        return 1
    fi
    shift
    #   check
    local f
    for f in "$@"
    do
        local num=$(wc -l < "${f}")
        if [ "$?" != 0 ]
        then
            echo "${0}: ${FUNCNAME[0]} failed: \"wc -l ${f}\"" 1>&2
            return 1
        fi
        if [ "${num}" != "${first_num}" ]
        then
            echo "${0}: ${FUNCNAME[0]} failed: different line numbers: " 1>&2
            echo "line number ${first_num} for ${first_file}" 1>&2
            echo "line number ${num} for ${f}" 1>&2
            return 1
        fi
    done
    return 0
}

#   Usage:
#       replace_config config key value
#   Function:
#       Replace "^${key}=.*" to "${key}=\"${value}\"" in config file.
function replace_config
{
    if [ "$#" -lt 3 ]
    then
        echo "${0}: ${FUNCNAME[0]}: config key value" 1>&2
        exit 1
    fi
    local config="${1}"
    local key="${2}"
    local value="${3}"
    #   replace
    #   TODO: support escape replace
    safe_execute "sed -i 's/^${key}=.*/${key}=\"${value}\"/g' '${config}'"
    return 0
}

#   get relative path
function get_rel_path
{
    if [ "$#" -lt 3 ]
    then
        echo "${FUNCNAME[0]} src_path tgt_path rel_path" 1>&2
        echo "Function:" 1>&2
        echo "    Set the relative path of target path given source path." 1>&2
        echo "Example:" 1>&2
        echo "[in]  src_path: \"/home/part1/part2\"" 1>&2
        echo "[in]  tgt_path: \"/work/part4/part5\"" 1>&2
        echo "[out] rel_path: \"../../part4/part5\"" 1>&2
        exit 1
    fi
    local src_path="${1}"
    local tgt_path="${2}"
    local __rel_path="$(perl -MFile::Spec -e 'print File::Spec->abs2rel("'"${tgt_path}"'","'"${src_path}"'")')"
    eval ${3}="${__rel_path}"
    return 0
}

#   get date str
function get_date_str
{
    if [ "$#" -lt 1 ]
    then
        echo "${FUNCNAME[0]} date_str" 1>&2
        exit 1
    fi
    local __date_str=""
    local host_name="$(hostname)"
    if [ "${host_name}" == "mtstudio" ] || [ "${host_name}" == "mad" ]
    then
        __date_str="today"
    else
        safe_execute "sleep 3"
        __date_str="$(date +%Y%m%d%H%M%S)"
    fi
    echo "get_date_str: ${__date_str}"
    eval ${1}="${__date_str}"
    return 0
}

