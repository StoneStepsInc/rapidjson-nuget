## RapidJSON Nuget Package

This project builds an RapidJSON Nuget package with RapidJSON
header files.

Visit RapidJSON website for additional information about the
RapidJSON project documentation:

https://github.com/Tencent/rapidjson

## Package Configuration

RapidJSON is an header-only library and does not contain any static
or dynamic library modules.

CMake is configured to build RapidJSON with the following feature
options:

  * `RAPIDJSON_BUILD_DOC=OFF`
  * `RAPIDJSON_BUILD_EXAMPLES=OFF`
  * `RAPIDJSON_BUILD_TESTS=OFF`
  * `RAPIDJSON_BUILD_THIRDPARTY_GTEST=OFF`
  * `RAPIDJSON_HAS_STDSTRING=ON`

Following RapidJSON flags will be enabled in the Visual Studio
project including this package:

  * `RAPIDJSON_HAS_CXX11_RVALUE_REFS`
  * `RAPIDJSON_HAS_STDSTRING`
  * `RAPIDJSON_HAS_CXX11_NOEXCEPT`

## RapidJSON Changes

RapidJSON source that was used to create this package contains a few
changes applied in patches described in this section against the
RapidJSON release indicated in the package version.

### `01-c++17.patch`

This patch removes inheritance from `std::iterator` for one
of RapidJSON iterators because this pattern has been deprecated
in C\+\+17.

### `02-cmake-install-prefix.patch`

This patch removes the explicit `CMAKE_INSTALL_PREFIX` reference
in CMake configuration, which makes the `--prefix` option ignored
when `cmake --install` is being run.

## Building a Nuget Package

This project can build a Nuget package for RapidJSON either locally
or via a GitHub workflow. In each case, following steps are taken.

  * RapidJSON source archive is downloaded from RapidJSON's website and
    its SHA-256 signature is verified.

  * The source is patched to build in Visual C++ 2022.

  * CMake is used to generate Visual Studio project files.

  * VS2022 Community Edition is used to build RapidJSON libraries
    locally and Enterprise Edition to build libraries on GitHub.

  * Build artifacts for all platforms and configurations are
    collected in staging directories under `nuget/build/native`.

  * `nuget.exe` is used to package staged files with the first
    three version components used as a RapidJSON version and the last
    version component used as a package revision. See _Package
    Version_ section for more details.

  * The Nuget package built on GitHub is uploaded to [nuget.org][].
    The package built locally is saved in the root project
    directory.

## Package Version

### Package Revision

Nuget packages lack package revision and in order to repackage
the same upstream software version, such as RapidJSON v1.1.0,
the 4th component of the Nuget version is used to track the
Nuget package revision.

Nuget package revision is injected outside of the Nuget package
configuration, during the package build process, and is not
present in the package specification file.

Specifically, `nuget.exe` is invoked with `-Version=1.1.0.123`
to build a package with the revision `123`.

### Version Locations

RapidJSON version is located in a few places in this repository and
needs to be changed in all of them for a new version of RapidJSON.

  * nuget/StoneSteps.RapidJSON.VS2022.Static.nuspec (`version`)
  * devops/make-package.bat (`PKG_VER`, `PKG_REV`, `RAPIDJSON_SHA256`)
  * .github/workflows/build-nuget-package.yml (`name`, `PKG_VER`,
    `PKG_REV`, `RAPIDJSON_FNAME`, `RAPIDJSON_DNAME`, `RAPIDJSON_SHA256`)

`RAPIDJSON_SHA256` ia a SHA-256 checksum of the RapidJSON package
file and needs to be updated when a new version of RapidJSON is
released.

In the GitHub workflow YAML, `PKG_REV` must be reset to `1` (one)
every time RapidJSON version is changed. The workflow file must be
renamed with the new version in the name. This is necessary because
GitHub maintains build numbers per workflow file name.

For local builds package revision is supplied on the command line
and should be specified as `1` (one) for a new version of RapidJSON
and incremented with every package release for the same upstream
version.

### GitHub Build Number

Build number within the GitHub workflow YAML is maintained in an
unconventional way because of the lack of build maturity management
between GitHub and Nuget.

For example, using build management systems, such as Artifactory,
every build would generate a Nuget package with the same version
and package revision for the upcoming release and build numbers
would be tracked within the build management system. A build that
was successfully tested would be promoted to the production Nuget
repository without generating a new build.

Without a build management system, the GitHub workflow in this
repository uses the pre-release version as a surrogate build
number for builds that do not publish packages to nuget.org,
so these builds can be downloaded and tested before the final
build is made and published to nuget.org. This approach is not
recommended for robust production environments because even
though the final published package is built from the exact
same source, the build process may still potentially introduce 
some unknowns into the final package (e.g. build VM was updated).

## Building Package Locally

You can build a Nuget package locally with `make-package.bat`
located in `devops`. This script expects VS2022 Community Edition
installed in the default location. If you have other edition of
Visual Studio, edit the file to use the correct path to the
`vcvarsall.bat` file.

Run `make-package.bat` from the repository root directory with a
package revision as the first argument. There is no provision to
manage build numbers from the command line and other tools should
be used for this.

## Sample Application

A Visual Studio project is included in this repository under
`sample-rapidjson` to test the Nuget package built by this
project.

In order to build `sample-rapidjson.exe`, open Nuget Package manager
in the solution and install either the locally-built Nuget package
or the one from [nuget.org][].

[nuget.org]: https://www.nuget.org/packages/StoneSteps.RapidJSON.VS2022.Static/
