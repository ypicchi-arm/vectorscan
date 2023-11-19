# About Vectorscan

A fork of Intel's Hyperscan, modified to run on more platforms. Currently ARM NEON/ASIMD
is 100% functional, and Power VSX are in development. ARM SVE2 support is in ongoing with
access to hardware now. More platforms will follow in the future.

Vectorscan will follow Intel's API and internal algorithms where possible, but will not
hesitate to make code changes where it is thought of giving better performance or better
portability. In addition, the code will be gradually simplified and made more uniform and
all architecture specific -currently Intel- #ifdefs will be removed and abstracted away.

# Why was there a need for a fork?

Originally, the ARM porting was intended to be merged into Intel's own Hyperscan, and relevant 
Pull Requests were made to the project for this reason. Unfortunately, the
PRs were rejected for now and the forseeable future, thus we have created Vectorscan for 
our own multi-architectural and opensource collaborative needs.

The recent license change of Hyperscan makes Vectorscan even more needed.

# What is Hyperscan?

Hyperscan is a high-performance multiple regex matching library. It follows the
regular expression syntax of the commonly-used libpcre library, but is a
standalone library with its own C API.

Hyperscan uses hybrid automata techniques to allow simultaneous matching of
large numbers (up to tens of thousands) of regular expressions and for the
matching of regular expressions across streams of data.

Vectorscan is typically used in a DPI library stack, just like Hyperscan.

# Installation

## Debian/Ubuntu

On recent Debian/Ubuntu systems, vectorscan should be directly available for installation:

```
$ sudo apt install libvectorscan5
```

Or to install the devel package you can install `libvectorscan-dev` package:

```
$ sudo apt install libvectorscan-dev
```

## Fedora

TBD

## Suse

TBD

## Alpine

TBD

## Other

# Build Instructions

The build system has recently been refactored to be more modular and easier to extend. For that reason,
some small but necessary changes were made that might break compatibility with how Hyperscan was built.

## Common Dependencies

In order to build on Debian/Ubuntu make sure you install the following build-dependencies

```
$ sudo apt build-essential cmake ragel pkg-config libsqlite3-dev libpcap-dev
```

## Configure & build

In order to configure with `cmake` first create and cd into a build directory:

```
$ mkdir build
$ cd build
```

Then call `cmake` from inside the `build` directory:

```
$ cmake ../
```

Common options for Cmake are:

* `-DBUILD_STATIC_LIBS=On/Off` Build static libraries
* `-DBUILD_SHARED_LIBS=On/Off` Build shared libraries
* `-DCMAKE_BUILD_TYPE=[Release|Debug|RelWithDebInfo|MinSizeRel]` Configure build type and determine optimizations and certain features, for examples, Fat runtimes are not compatible with Debug mode at the moment.

And then you can run `make` in the same directory, if you have a multi-core system with `N` cores, running

```
$ make -j <N>
```

will speed up the process. If all goes well, you should have the vectorscan library

## Native CPU detection

Native CPU detection is off by default, however it is possible to build a performance-oriented non-fat library tuned to your CPU, as detected by the compiler:

```
$ cmake ../
```


## Instructions for Intel/AMD CPUs

## Instructions for Arm 64-bit CPUs

## Instructions for Power8/Power9/Power10 CPUs


## Fat Runtime (Intel/AMD 64-bit & Arm 64-bit Only)


# License

Vectorscan follows a BSD License like the original Hyperscan (up to 5.4).

Vectorscan continues to be an open source project and we are committed to keep it that way.
See the LICENSE file in the project repository.

## Hyperscan License Change after 5.4

According to 
[Accelerate Snort Performance with Hyperscan and Intel Xeon Processors on Public Clouds](https://networkbuilders.intel.com/docs/networkbuilders/accelerate-snort-performance-with-hyperscan-and-intel-xeon-processors-on-public-clouds-1680176363.pdf) versions of Hyperscan later than 5.4 are
going to be closed-source:

> The latest open-source version (BSD-3 license) of Hyperscan on Github is 5.4. Intel conducts continuous internal
> development and delivers new Hyperscan releases under Intel Proprietary License (IPL) beginning from 5.5 for interested
> customers. Please contact authors to learn more about getting new Hyperscan releases.

# Versioning

The `master` branch on Github will always contain the most recent stable release of
Hyperscan. Each version released to `master` goes through QA and testing before
it is released; if you're a user, rather than a developer, this is the version
you should be using.

Further development towards the next release takes place on the `develop`
branch. All PRs are first made against the develop branch and if the pass the [Vectorscan CI](https://buildbot-ci.vectorcamp.gr/#/grid), then they get merged. Similarly with PRs from develop to master.

# Compatibility with Hyperscan

Vectorscan aims to be ABI and API compatible with the last open source version of Intel Hyperscan 5.4.
After careful consideration we decided that we will **NOT** aim to achieving compatibility with later Hyperscan versions 5.5/5.6 that have extended Hyperscan's API.
If keeping up to date with latest API of Hyperscan, you should talk to Intel and get a license to use that.
However, we intend to extend Vectorscan's API with user requested changes or API extensions and improvements that we think are best for the project.

# Contributions

The official homepage for Vectorscan is at [www.github.com/VectorCamp/vectorscan](https://www.github.com/VectorCamp/vectorscan).

# Vectorscan Development

All development of Vectorscan is done in public. 

# Original Hyperscan links
For reference, the official homepage for Hyperscan is at [www.hyperscan.io](https://www.hyperscan.io).

# Hyperscan Documentation

Information on building the Hyperscan library and using its API is available in
the [Developer Reference Guide](http://intel.github.io/hyperscan/dev-reference/).

And you can find the source code [on Github](https://github.com/intel/hyperscan).

For Intel Hyperscan related issues and questions, please follow the relevant links there.