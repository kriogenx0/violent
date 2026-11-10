# AGENTS.md — Violent

Violent is a JUCE 8 synth/sampler plugin (AU, VST3, and a Standalone macOS app) in C++17, built with CMake and Ninja. JUCE is pulled with `FetchContent`. Open follow-ups live in [BACKLOG.md](BACKLOG.md).

## Commands

- `make setup`: install cmake/ninja via Homebrew if missing, then configure and build.
- `make dev`: Debug build, kill any running instance, and launch the Standalone app (`build/Violent_artefacts/Debug/Standalone/Violent.app`).
- `make build`: compile everything in `build/` (configure first with `make configure`).
- `make install`: install the AU and VST3 into `~/Library/Audio/Plug-Ins`.
- `make close`: quit the running Standalone app.
- Tests: `make build`, then run `./build/ViolentTests_artefacts/Debug/ViolentTests`.
- `make help` lists every target.

## Layout

- `Source/PluginProcessor.*`, `Source/PluginEditor.*`: the JUCE plugin entry points.
- `Source/SoundProcessing/`: synth, sampler, filter, effects, modulators, MIDI modifiers.
- `Source/Views/`: rack, mixer, generator, effect, modulator, and spectrum views; shared widgets are in `SharedComponents.*`.
- `Source/*Chain.h`: generator, FX, and modulator chains.
- `Tests/`: JUCE `UnitTest` suites compiled into the `ViolentTests` console app.
- `Assets/`: icon and bundled resources.

## Conventions

- Register new `.cpp` files in `CMakeLists.txt` (`target_sources` for the plugin and, when tested, `ViolentTests`).
- Fix compiler warnings in project code; JUCE's own deprecation warnings are already suppressed.
- Nothing in the audio callback may allocate, lock, or log.
- Read `BACKLOG.md` before build or tooling changes. `build/` may be pinned to a local JUCE copy, so don't `make clean` when offline.
- Update `BACKLOG.md` when you finish or discover a follow-up.

## Design principles

Follow the [design principles](docs/rules/design-principles.md) for any UI work.

## After changing code

1. `make dev`
2. `./build/ViolentTests_artefacts/Debug/ViolentTests`

Commit messages are one short, imperative line, like the existing history.
