#!/bin/bash

# ------ default values
build_vectorscan=false
install_vectorscan=false
build_snort=false
install_snort=false
run_snort=true
WS=/home/`whoami`/code/snort
version_tag=""
pcap=defcon.pcap
thread=4
verbose=false
compact_report=false
repeat_run=1
search_engine=ac_bnfa
neon_only=false
sve2=false
debug=false
extra_cmake_arg="-DFAT_RUNTIME=0"
dir_suffix=""

# ------ updates default based on the environment


# ------ parse
usage="\
Usage: $0 [OPTION]... [VAR=VALUE]...
--build-snort
--install-snort
--build-vectorscan
--install-vectorscan
--no-run
--verbose/-v
--compact-report
--neon-only
--sve2
--debug
--workspace=<${WS}>
--version-tag=<${version_tag}>
--pcap=<${pcap}>
--thread=<${thread}>
--repeat=<${repeat_run}>
--search-engine=<${search_engine}>
--dir-suffix=${dir_suffix}>
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
        --build-snort)
            build_snort=true
            ;;
        --build-vectorscan)
            build_vectorscan=true
            ;;
        --install-snort)
            install_snort=true
            ;;
        --install-vectorscan)
            install_vectorscan=true
            ;;
        --no-run)
            run_snort=false
            ;;
        --version-tag=*)
	    version_tag=$optarg
            ;;
        --pcap=*)
	    pcap=$optarg
            ;;
        --thread=*)
	    thread=$optarg
            ;;
        --verbose|-v)
	    verbose=true
            ;;
        --compact-report)
	    compact_report=true
            ;;
        --repeat=*)
	    repeat_run=$optarg
            ;;
        --search-engine=*)
	    search_engine=$optarg
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
	    --dir-suffix=*)
            dir_suffix=$optarg
	        ;;
        *)
            echo "Invalid option '$1'.  Try $0 --help to see available options."
            exit 1
            ;;
    esac
    shift
done

# ------ check config

vectorscan_dir=${WS}/vectorscan
snort_dir=${WS}/snort
build_dir_name=build_${HOSTNAME}_${version_tag}

if $neon_only; then
	extra_cmake_arg="$extra_cmake_arg -DBUILD_SVE2_BITPERM=0 -DBUILD_SVE=0 -DBUILD_SVE2=0"
	build_dir_name=${build_dir_name}_neon_only
	snort_install_dir="/usr/local/snort_neon"
else
	if $sve2; then
		extra_cmake_arg="$extra_cmake_arg -DBUILD_SVE2_BITPERM=1 -DBUILD_SVE2=1"
	    build_dir_name=${build_dir_name}_sve2
        snort_install_dir="/usr/local/snort_sve2"
	else
		extra_cmake_arg="$extra_cmake_arg -DBUILD_SVE=1 -DBUILD_SVE2_BITPERM=0 -DBUILD_SVE2=0"
        snort_install_dir="/usr/local/snort_sve"
	fi
fi

if $debug; then
	extra_cmake_arg="$extra_cmake_arg -DCMAKE_BUILD_TYPE=Debug"
else
	extra_cmake_arg="$extra_cmake_arg -DCMAKE_BUILD_TYPE=Release"
fi

build_dir_name=${build_dir_name}${dir_suffix}

vectorscan_build_dir=${vectorscan_dir}/${build_dir_name}
snort_build_dir=${snort_dir}/${build_dir_name}


if $install_snort && [[ ! -d $snort_build_dir ]] && [[ ! $build_snort ]]; then
	echo "build dir not found : ${snort_build_dir}"
	exit 1
fi
snort_install_dir="${snort_install_dir}_${version_tag}${dir_suffix}"


# ------ execute

cd ${WS}
sudo ldconfig
cd ${WS}/tests/snort3-perf

if $build_vectorscan; then
	echo ""
	echo "Building vectorscan..."
	echo ""
	pushd $vectorscan_dir
	build_tag=""
	if $neon_only; then
		build_tag="${build_tag} --neon-only"
	fi
	if $sve2; then
		build_tag="${build_tag} --sve2"
	fi
	if $debug; then
		build_tag="${build_tag} --debug"
	fi
	./build_vectorscan.sh --workspace=${WS} --dir-suffix=${dir_suffix} --version-tag=${version_tag} $build_tag
	if [[ $? -ne 0 ]]; then
		echo "failed to build vectorscan"
		popd
		exit 2
	fi
	popd
fi

if $install_vectorscan; then
	echo ""
	echo "Installing vectorscan..."
	echo ""
	pushd $vectorscan_build_dir
	if [[ $? -ne 0 ]]; then
		echo "failed to find vectorscan's build dir"
		popd
		exit 2
	fi
	sudo make install -j 8
	if [[ $? -ne 0 ]]; then
		echo "failed installing vectorscan"
		popd
		exit 2
	fi
	popd
fi

if $build_snort; then
	echo ""
	echo "Building snort..."
	echo ""
	export LD_LIBRARY_PATH=/usr/local/lib:/lib:/usr/lib
	export CPATH=/usr/local/include:/usr/include
	export CFLAGS="-march=native -mtune=native"
	export CXXFLAGS="-march=native -mtune=native"
	pushd ${snort_dir}
	#checkout only if a tag is set
	if [[ -n $version_tag ]]; then
		echo "checking out $version_tag"
		git checkout $version_tag
		if [[ $? -ne 0 ]]; then
			echo "failed to git checkout"
			popd
			exit 2
		fi
	fi
	${WS}/snort/configure_cmake.sh --prefix=${snort_install_dir} --enable-tcmalloc --build-type=Release --builddir=${snort_build_dir}
	if [[ $? -ne 0 ]]; then
		echo "failed configuring snort"
		popd
		exit 2
	fi
	make -C ${snort_build_dir} -j 8
	if [[ $? -ne 0 ]]; then
		echo "failed building snort"
		popd
		exit 2
	fi
	popd
fi

if $install_snort; then
	echo ""
	echo "Installing snort..."
	echo ""
	if [[ ! -d ${snort_build_dir} ]]; then
		echo "failed to find snort's build dir"
		exit 2
	fi
	pushd ${WS}/snort/
	sudo make -C ${snort_build_dir} install
	if [[ $? -ne 0 ]]; then
		echo "failed installing snort"
		popd
		exit 2
	fi
	popd
fi

if $run_snort; then
	echo ""
	echo "Running benchmark on ${snort_install_dir}/bin/snort ..."
	echo ""
	if [[ ! -d ${snort_install_dir} ]]; then
		echo "failed to find snort's install dir"
		exit 2
	fi
	export SNORT_LUA_PATH=/usr/local/etc/snort
	export SNORT_ODP=${WS}/tests/snort3-perf
	export SNORT3_MAX_RULES=max.rules
	export SNORT3_MIN_RULES=max.rules
	config=detect-max.lua
	output_filename=${HOSTNAME}_output.txt
	for ((i=0; i<$repeat_run; i++));
		do
		if $verbose; then
			${snort_install_dir}/bin/snort -c ${config} --lua "search_engine.search_method = \"${search_engine}\"" --pcap-filter ${pcap} --pcap-loop $((${thread} * 4)) --snaplen 9000 --max-packet-threads ${thread} --daq dump --daq-var output=none -H --include-path /usr/local/etc/snort --pcap-dir ${WS}/pcap --plugin-path /usr/local/lib64 -Q --warn-conf-strict -v;
		else
			${snort_install_dir}/bin/snort -c ${config} --lua "search_engine.search_method = \"${search_engine}\"" --pcap-filter ${pcap} --pcap-loop $((${thread} * 4)) --snaplen 9000 --max-packet-threads ${thread} --daq dump --daq-var output=none -H --include-path /usr/local/etc/snort --pcap-dir ${WS}/pcap --plugin-path /usr/local/lib64 -Q --warn-conf-strict > $output_filename
			if $compact_report; then
				cat $output_filename | grep -A 4 "timing" | sed -E 's/[^0-9:.]{2,}//g' | sed 's/: //g' | tr '\r\n' ' ' | sed 's/.$/\n/' | cut -d ' ' --complement -f1,2 | sed 's/ /\t/g'
			else
				cat $output_filename | grep -A 4 "timing" | sed -E 's/  +/  /g'
			fi
			rm $output_filename
		fi
	done
fi
