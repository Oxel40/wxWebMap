# GitHub Actions in wxWebMap and other I-CONIC repositories
Actions consists of workflows who intern defines what jobs/steps needs to be run.

## Current status
Currently is the repository `Oxel40/wxWebMap` has a working setup with github actions for building and creating releases. On each push to the master branch, wxWebMap will be built and packaged into a release for windows-release and windows-debug. On each pull request will the build be run for both windows-debug and windows-release as a check that needs to be passed in order to merge the pull request.

Only some minor changes are needed (see [Steps to setup](#steps-to-setup)) to get these workflows to work with the official wxWebMap and potentially other repositories like API and studio.

Adding Linux (Ubuntu) and MacOS is a trivial change to the build scripts, but some work is required to get these fully up and running (should only be a dependency issue for Linux, see bellow in [Notes](#notes)).

## Steps to setup

1. Add buildPresets to `CMakePresets.json`
2. Add `vcpkg.json`
3. Copy workflow files found in `.github/workflows/` to the desired repository.

### Build Presets
As mentioned, buildPresets needs to be added to `CMakePresets.json` (used by `lukka/run-cmake` etc. and if you run cmake from the terminal). This can (as in this case) be as simple as modifying the `CMakePresets.json` file like this:

```json
{
  "version": 3,
  "cmakeMinimumRequired": {
    ...
  },
  "configurePresets": [
    ...
  ],
  "buildPresets": [
    {
      "name": "windows-x64-debug",
      "configurePreset": "windows-x64-debug"
    },
    {
      "name": "windows-x64-release",
      "configurePreset": "windows-x64-release"
    },
    {
      "name": "linux-x64-debug",
      "configurePreset": "linux-x64-debug"
    },
    {
      "name": "linux-x64-release",
      "configurePreset": "linux-x64-release"
    }
  ]
}
```

### `vcpkg.json`
on the root of the repository (or rather cmake project), add a `vcpkg.json` that describes all dependencies needed to build the project. This is how that file could look like for wxWebMap:

```json
{
    "name": "wxwebmap",
    "version-string": "1.3.1",
    "dependencies": [
		{
			"name": "wxwidgets",
			"default-features": true,
			"features": [ "webview" ]
		}
    ]
}
```

### Workflows
There are 4 different workflow files created:

- `build-and-release.yml`: triggers when a push to main is done, calls on `iconic-cmake-build.yml` and then `iconic-release.yml`.
- `build.yml`: triggers on a pull request (runs like a check before allowing the PR to merge), calls on `iconic-cmake-build.yml`.
- `iconic-cmake-build.yml`: uses `lukka/run-cmake` and friends to install dependencies and build the project before generating a zip file of the built assets and creates an artifact.
- `iconic-release.yml`: uses `cURL` to create a release and upload artifacts to the release.

> All these workflows can be triggered by hand in the "Actions" tab on GitHub.
> 
> **NOTE:** workflow files needs to be located in the `.github/workflows` directory to be able to be triggered.

## Suggested setup
Like with the files above can workflows (or rather workflow files) be reused, they doesn't even have to be in the same repository as the calling workflow (as long as they are located in a public repository). Thus is my suggested setup as follows:

- A public repository, e.g. `I-CONIC-Vision-AB/workflows`, where we can put larger reusable workflow files (`iconic-cmake-build.yml` and `iconic-release.yml`). Making it easier to update these workflows across all repositories.
- Workflow files in all repositories of interest (e.g. `I-CONIC-Vision-AB/wxWebMap`) which is only responsible for acting on triggers (e.g. pull requests or pushes to main/master) that calls upon workflows defined in `I-CONIC-Vision-AB/workflows` (See `build.yml` and `build-and-release.yml`).

## Problems encountered and potential problems
- No public action for creating a release seems to work (there is no official one from GitHub). The solution has been to use cURL to send requests to GitHubs REST API.
- Problems with wxWidgets on ubuntu-latest, there fore isn't ubuntu-latest included in the os'es in `iconic-cmake-build.yml`. This seems to be a quite fresh issue related to [this issue](https://github.com/microsoft/vcpkg/issues/28833), with the only difference being that wxWidgets compiles with WebView specified in the `vcpkg.json`.
- Time to compile dependencies. This could potentially be a problem for the API and studio repos, but it needs to be further explored.
- The potential need for `--clean-after-build`, can be fixed with some slight tweaks to the CMake build workflow. This might be needed for API and studio where the build trees and downloads from all dependencies might take up all available space on the action worker.
- RAM needed to compile (isn't 8GB enough?), can make use of page/swap file with action `al-cheb/configure-pagefile-action@v1.3` for windows workers and `pierotofy/set-swap-space@master` for ubuntu workers.
- CUDA, can be installed with action `Jimver/cuda-toolkit@v0.2.8`. Needed for API and studio.

## Notes
Just some stuff worth mentioning.

### vcpkg as submodule
To keep the development and remote build environment consistent does `lukka/run-vcpkg` recommend using vcpkg as a sub-module, resulting in the same git commit being the base for both local and remote builds. This might not be desired for us and thus is the latest version of `I-CONIC-Vision-AB/vcpkg` used instead by cloning the repository in `iconic-cmake-build.yml`.

### Adding a `vcpkg.json` to all repositories
Adding a `vcpkg.json` file with all dependencies to the root of each repository might be a good thing to do. This allows us to unify dependency installation, instead of using the two current different shell scripts located in the vcpkg repository. Having a `vcpkg.json` in the root of the repository will (in my experience) integrate with visual studio who automatically installs (using vcpkg) and checks for all dependencies listed in this file, and running e.g. `cmake -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake" --preset linux-x64-release` will also install all dependencies specified in `vcpkg.json`. I'd argue that this approach makes our dependency management easier to handle (no need to tell everyone to install a new library if the dependencies change, just commit and push the `vcpkg.json`) and more declarative.

### Reduce the amount of boost dependencies
Boost is a huge library with many many different components, many of which we don't use. A good idea might be to only install the necessary parts of the boost library, this is mainly to reduce build time (or rather dependency build time) in the future GitHub actions responsible for building API and studio. Some work has already started on moving the usage of e.g. `boost::smart_ptr` to `std::smart_ptr`, with could further reduce the dependency on boost libraries.
