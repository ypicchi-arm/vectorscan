neon_only=false
sve2=false
version_tag=""
WS=/home/`whoami`/code/snort
run_unit_test=false
dir_suffix=""
grep_string=""


# ------ parse
usage="\
Usage: $0 [OPTION]... [VAR=VALUE]...
--neon-only
--sve2
--unit
--workspace=<${WS}>
--dir-suffix=<${dir_suffix}>
--version-tag=<${version_tag}>
--version-tag=<${grep_string}>
"

while [ $# -ne 0 ]; do
    case "$1" in
        *=*) optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
        *) optarg= ;;
    esac

    case "$1" in
        --help|-h)
            echo "${usage}" 1>&2
            exit 1
            ;;
        --workspace=*)
            WS=$optarg
            ;;
        --neon-only)
            neon_only=true
            ;;
        --sve2)
            sve2=true
            ;;
        --unit)
            run_unit_test=true
            ;;
	    --dir-suffix=*)
            dir_suffix=$optarg
	        ;;
        --version-tag=*)
	        version_tag=$optarg
            ;;
        --grep=*)
	        grep_string=$optarg
            ;;
        *)
            echo "Invalid option '$1'.  Try $0 --help to see available options."
            exit 1
            ;;
    esac
    shift
done

build_dir_name=build_${HOSTNAME}_${version_tag}

if $neon_only; then
	build_dir_name=${build_dir_name}_neon_only
else
        if $sve2; then
        	build_dir_name=${build_dir_name}_sve2
        fi
fi

build_dir_name=${build_dir_name}${dir_suffix}

vectorscan_build_dir=${WS}/vectorscan/${build_dir_name}

pushd "${vectorscan_build_dir}/bin"
if [[ $? -ne 0 ]]; then
	exit 2
fi
if $run_unit_test; then
	if [ -n "${grep_string}" ]; then
		./unit-internal | grep -i "$grep_string" -A 1 -B 1
	else
		./unit-internal
	fi
else
	if [ -n "${grep_string}" ]; then
		./benchmarks | grep -i "$grep_string"
	else
		./benchmarks
	fi
fi
popd