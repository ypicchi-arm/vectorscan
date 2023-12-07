neon_only=false
sve2=false
version_tag=""
extra_cmake_arg="-DFAT_RUNTIME=0"
debug=false
WS=/home/`whoami`/code/snort
dir_suffix=""



# ------ parse
usage="\
Usage: $0 [OPTION]... [VAR=VALUE]...
--neon-only
--sve2
--workspace=<${WS}>
--dir-suffix=<${dir_suffix}>
--version-tag=<${version_tag}>
--fat-runtime
--debug
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
        --debug)
	        debug=true
			;;
        --fat-runtime)
			extra_cmake_arg="-DFAT_RUNTIME=1"
            ;;
        --dir-suffix=*)
            dir_suffix=$optarg
	        ;;
        --version-tag=*)
	        version_tag=$optarg
            ;;
        *)
            echo "Invalid option '$1'.  Try $0 --help to see available options."
            exit 1
            ;;
    esac
    shift
done

vectorscan_dir=${WS}/vectorscan
build_dir_name=build_${HOSTNAME}_${version_tag}

if $neon_only; then
    extra_cmake_arg="$extra_cmake_arg -DBUILD_SVE2_BITPERM=0 -DBUILD_SVE=0 -DBUILD_SVE2=0"
    build_dir_name=${build_dir_name}_neon_only
else
    if $sve2; then
        extra_cmake_arg="$extra_cmake_arg -DBUILD_SVE=1 -DBUILD_SVE2_BITPERM=1 -DBUILD_SVE2=1"
        build_dir_name=${build_dir_name}_sve2
    else
        extra_cmake_arg="$extra_cmake_arg -DBUILD_SVE=1 -DBUILD_SVE2_BITPERM=0 -DBUILD_SVE2=0"
    fi
fi

if $debug; then
	extra_cmake_arg="$extra_cmake_arg -DCMAKE_BUILD_TYPE=Debug"
else
	extra_cmake_arg="$extra_cmake_arg -DCMAKE_BUILD_TYPE=Release"
fi

build_dir_name=${build_dir_name}${dir_suffix}

vectorscan_build_dir=${vectorscan_dir}/${build_dir_name}

pushd $WS
sudo ldconfig
popd

export LD_LIBRARY_PATH=/usr/local/lib:/lib:/usr/lib
export CPATH="" #empty on purpose
export CFLAGS="-march=native -mtune=native"
export CXXFLAGS="-march=native -mtune=native"

pushd $vectorscan_dir
#checkout only if a tag is set
if [[ -n $version_tag ]]; then
	git checkout $version_tag
	if [[ $? -ne 0 ]]; then
		echo "failed to git checkout"
		popd
		exit 2
	fi
fi
mkdir $vectorscan_build_dir
cd $vectorscan_build_dir &&
echo "cmake -DBUILD_STATIC_LIBS=True -DBUILD_SHARED_LIBS=True $extra_cmake_arg $vectorscan_dir" &&
cmake -DBUILD_STATIC_LIBS=True -DBUILD_SHARED_LIBS=True $extra_cmake_arg $vectorscan_dir &&
make -j 8
status=$?
popd
exit $status
