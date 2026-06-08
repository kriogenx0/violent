BUILD_DIR   := build
CMAKE       := cmake
NINJA       := ninja
AU_DEST     := $(HOME)/Library/Audio/Plug-Ins/Components
VST3_DEST   := $(HOME)/Library/Audio/Plug-Ins/VST3

export MACOSX_DEPLOYMENT_TARGET := 13.0

.PHONY: all setup configure build dev install install-au install-vst3 install-standalone run clean dependency-check help

## Default: build (Debug) + launch standalone
all: dependency-check dev

## Install build dependencies via Homebrew, then configure and build
setup:
	@command -v brew >/dev/null 2>&1 || { echo "Homebrew not found. Install it from https://brew.sh"; exit 1; }
	@command -v cmake >/dev/null 2>&1 || brew install cmake
	@command -v ninja >/dev/null 2>&1 || brew install ninja
	@command -v git   >/dev/null 2>&1 || xcode-select --install
	$(MAKE) configure
	$(MAKE) build

## Step 1 – Generate the build system with CMake + Ninja
configure:
	$(CMAKE) -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Debug

## Step 1 (Release) – same but optimised
configure-release:
	$(CMAKE) -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Release

## Step 2 – Compile everything
build: $(BUILD_DIR)/build.ninja
	$(CMAKE) --build $(BUILD_DIR) --parallel

## Shorthand: configure then build in one step (Debug)
setup-and-build:
	$(MAKE) configure
	$(MAKE) build

## Shorthand: configure then build in one step (Release)
setup-and-build-release:
	$(MAKE) configure-release
	$(MAKE) build

## Install AU to ~/Library/Audio/Plug-Ins/Components
install-au: build
	mkdir -p "$(AU_DEST)"
	cp -r "$(BUILD_DIR)/Violent_artefacts/AU/Violent.component" "$(AU_DEST)/"
	@echo "Installed AU → $(AU_DEST)/Violent.component"

## Install VST3 to ~/Library/Audio/Plug-Ins/VST3
install-vst3: build
	mkdir -p "$(VST3_DEST)"
	cp -r "$(BUILD_DIR)/Violent_artefacts/VST3/Violent.vst3" "$(VST3_DEST)/"
	@echo "Installed VST3 → $(VST3_DEST)/Violent.vst3"

## Install Standalone app to /Applications
install-standalone: build
	cp -r "$(BUILD_DIR)/Violent_artefacts/Standalone/Violent.app" /Applications/
	@echo "Installed Standalone → /Applications/Violent.app"

## Build debug version and launch the standalone app
dev: build
	open "$(BUILD_DIR)/Violent_artefacts/Standalone/Violent.app"

## Run the Standalone app directly from the build directory
run: build
	open "$(BUILD_DIR)/Violent_artefacts/Standalone/Violent.app"

## Install both AU and VST3
install: install-au install-vst3

## Delete the entire build directory (forces full reconfigure + rebuild)
clean:
	rm -rf $(BUILD_DIR)

## Check that required build tools are installed
dependency-check:
	@echo "Checking dependencies..."
	@command -v cmake  >/dev/null 2>&1 || { echo "  ✗ cmake not found  →  brew install cmake";  exit 1; }
	@command -v ninja  >/dev/null 2>&1 || { echo "  ✗ ninja not found  →  brew install ninja";  exit 1; }
	@command -v git    >/dev/null 2>&1 || { echo "  ✗ git not found    →  xcode-select --install"; exit 1; }
	@echo "  cmake  $(shell cmake --version | head -1)"
	@echo "  ninja  $(shell ninja --version)"
	@echo "  git    $(shell git --version)"
	@echo "All dependencies satisfied."

## Ensure build dir exists and CMake has been run
$(BUILD_DIR)/build.ninja:
	$(MAKE) configure

help:
	@echo ""
	@echo "Violent plugin – build targets"
	@echo "────────────────────────────────────────────"
	@echo "  make dev                   build (Debug) + launch standalone"
	@echo "  make setup                 install deps (brew) + configure + build"
	@echo "  make                       configure + build (Debug)"
	@echo "  make configure             run CMake (Debug)"
	@echo "  make configure-release     run CMake (Release)"
	@echo "  make build                 compile"
	@echo "  make setup-and-build       configure + build (Debug)"
	@echo "  make setup-and-build-release  configure + build (Release)"
	@echo "  make dependency-check          check cmake / ninja / git are installed"
	@echo "  make install-au            copy .component to ~/Library"
	@echo "  make install-vst3          copy .vst3 to ~/Library"
	@echo "  make install-standalone    copy .app to /Applications"
	@echo "  make run                   launch standalone app from build dir"
	@echo "  make install               install both AU and VST3"
	@echo "  make clean                 delete entire build/ directory"
	@echo ""
	@echo "Prerequisites"
	@echo "────────────────────────────────────────────"
	@echo "  brew install cmake ninja"
	@echo ""
	@echo "First build clones JUCE from GitHub (requires internet)."
	@echo ""
