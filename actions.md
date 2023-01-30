# GitHub Actions in wxWebMap and other I-CONIC repositories
Actions consists of workflows who intern defines what jobs/steps needs to be run.

## Steps to setup

1. Add buildPresets to `CMakePresets.json`
2. Copy workflow files found in `.github/workflows/` to the desired repository.

### Build Presets
As mentioned, buildPresets needs to be added to `CMakePresets.json` (used by `lukka/run-cmake` etc.). This can (as in this case) be as simple as modifying the `CMakePresets.json` file like this:

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

### Workflows
There are 4 different workflow files created:

- `build-and-release.yml`: triggers when a push to main is done, calls on `iconic-cmake-build.yml` and then `iconic-release.yml`.
- `build.yml`: triggers on a pull request (runs like a check before allowing the PR to merge), calls on `iconic-cmake-build.yml`.
- `iconic-cmake-build.yml`: uses `lukka/run-cmake` and friends to install dependencies and build the project before generating a zip file of the built assets and creates an artifact.
- `iconic-release.yml`: uses `cURL` to create a release and upload artifacts to the release.

> All workflows can be triggered by hand in the "Actions" tab on GitHub.
> 
> **NOTE:** workflow files needs to be located in the `.github/workflows` directory to be able to be triggered.

## Suggested setup
Like with the files above can workflows (or rather workflow files) be reused, they doesn't even have to be in the same repository as the calling workflow (as long as they are located in a public repository). Thus is my suggested setup as follows:

- A public repository, e.g. `I-CONIC-Vision-AB/workflows`, where we can put larger reusable workflow files (`iconic-cmake-build.yml` and `iconic-release.yml`). Making it easier to update these workflows across all repositories.
- Workflow files in all repositories of interest (e.g. `I-CONIC-Vision-AB/wxWebMap`) which is only responsible for acting on triggers (e.g. pull requests or pushes to main/master) that calls upon workflows defined in `I-CONIC-Vision-AB/workflows` (See `build.yml` and `build-and-release.yml`).

## Problems encountered and potential problems
...

- No public action for creating a release seems to work
- Problems with wxWidgets on ubuntu-latest, there fore isn't ubuntu-latest included in the os'es in `iconic-cmake-build.yml`
- Time to compile dependencies
- The potential need for `--clean-after-build`, can be fixed with some slight tweaks to the CMake build workflow
- RAM needed to compile (isn't 8GB enough?), can make use of page/swap file with action
- CUDA, can be installed with action

## Notes
Just some stuff worth mentioning.

### vcpkg as submodule
To keep the development and remote build environment consistent does `lukka/run-vcpkg` recommend using vcpkg as a sub-module, resulting in the same git commit being the base for both local and remote builds. This might not be desired for us and thus is the latest version of `I-CONIC-Vision-AB/vcpkg` used instead by cloning the repository in `iconic-cmake-build.yml`.
