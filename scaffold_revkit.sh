#!/bin/bash 

ROOT=$(dirname $0)

# Get Revkit path
export Revkit_PATH=/home/adam/Documents/revkit-1.3
if [ $(echo $PATH | grep ${Revkit_PATH} | wc -l) -eq 0 ]; then
	export PATH=$PATH:$Revkit_PATH
fi

function show_help {
    echo "Usage: $0 [-h] [-rqfRFcpd] [-L #] <filename>.scaffold"
    echo "    -r   Generate resource estimate (default)"
    echo "    -q   Generate QASM"
    echo "    -f   Generate flattened QASM"
    echo "    -R   Disable rotation decomposition"
    echo "    -T   Disable Toffoli decomposition"    
	  echo "    -l   Levels of recursion to run (default=1)"
    echo "    -F   Force running all steps"
    echo "    -c   Clean all files (no other actions)"
    echo "    -p   Purge all intermediate files (preserves specified output,"
    echo "         but requires recompilation for any new output)"
    echo "    -d   Dry-run; show all commands to be run, but do not execute"
}

# Parse opts
OPTIND=1         # Reset in case getopts has been used previously in the shell.
revkit=0
clean=0
dryrun=""
force=0
purge=0
res=0
rot=1
toff=1
targets=""
while getopts "h?cdfFpqrRTl:" opt; do
    case "$opt" in
    h|\?)
        show_help
        exit 0
        ;;
    c) clean=1
        ;;
	  d) dryrun="--dry-run"
		;;
    F) force=1
        ;;
    f) targets="${targets} flat"
        ;;
    p) purge=1
        ;;
    q) targets="${targets} qasm"
        ;;
    r) res=1
        ;;
    R) rot=0
        ;;
    T) toff=0
        ;;        
    l) targets="${targets} SQCT_LEVELS=${OPTARG}"
        ;;
    esac
done
shift $((OPTIND-1))
[ "$1" = "--" ] && shift

# Put resources at the end so it is easy to read
if [ ${res} -eq 1 ]; then
    targets="${targets} resources"
fi
# Don't purge until done
if [ ${purge} -eq 1 ]; then
    targets="${targets} purge"
fi
# Force first
if [ ${force} -eq 1 ]; then
    targets="clean ${targets}"
fi
# Default to resource estimate
if [ -z "${targets}" ]; then
    targets="resources"
fi

if [ $# -lt 1 ]; then 
    echo "Error: Missing filename argument" 
    show_help 
    exit 1 
fi 

filename=${1}
if [ ! -e ${filename} ]; then
    echo "${filename}: file not found"
    show_help
    exit 1
fi
dir="$(dirname ${filename})/"
file=$(basename ${filename} .scaffold)
cfile="${file}.*"

if [ $(egrep '^revkit.*{\s*' ${filename} | wc -l) -gt 0 ]; then
	revkit=1
	toff=1
	dir=""
fi

if [ ${clean} -eq 1 ]; then
	make -f $ROOT/scaffold/Scaffold_revkit.makefile ${dryrun} ROOT=$ROOT DIRNAME=${dir} FILENAME=${filename} FILE=${file} CFILE=${cfile} clean
    exit
fi

make -f $ROOT/scaffold/Scaffold_revkit.makefile ${dryrun} ROOT=$ROOT DIRNAME=${dir} FILENAME=${filename} FILE=${file} CFILE=${cfile} TOFF=${toff} Revkit=${revkit} ROTATIONS=${rot} ${targets}

exit 0
