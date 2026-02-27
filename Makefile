BUILD_DIR   := build
CMAKE       := cmake
NINJA       := ninja
AU_DEST     := $(HOME)/Library/Audio/Plug-Ins/Components
VST3_DEST   := $(HOME)/Library/Audio/Plug-Ins/VST3

.PHONY: all configure build install install-au install-vst3 clean wipe help

## Default: configure (if needed) then build
all: build

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

## Install both AU and VST3
install: install-au install-vst3

## Remove build artefacts but keep the CMake cache
clean:
	$(CMAKE) --build $(BUILD_DIR) --target clean

## Nuke the entire build directory (forces full reconfigure + rebuild)
wipe:
	rm -rf $(BUILD_DIR)

## Ensure build dir exists and CMake has been run
$(BUILD_DIR)/build.ninja:
	$(MAKE) configure

help:
	@echo ""
	@echo "Violent plugin – build targets"
	@echo "────────────────────────────────────────────"
	@echo "  make                       configure + build (Debug)"
	@echo "  make configure             run CMake (Debug)"
	@echo "  make configure-release     run CMake (Release)"
	@echo "  make build                 compile"
	@echo "  make setup-and-build       configure + build (Debug)"
	@echo "  make setup-and-build-release  configure + build (Release)"
	@echo "  make install-au            copy .component to ~/Library"
	@echo "  make install-vst3          copy .vst3 to ~/Library"
	@echo "  make install               install both AU and VST3"
	@echo "  make clean                 remove compiled objects"
	@echo "  make wipe                  delete entire build/ directory"
	@echo ""
	@echo "Prerequisites"
	@echo "────────────────────────────────────────────"
	@echo "  brew install cmake ninja"
	@echo ""
	@echo "First build clones JUCE 7.0.9 from GitHub (requires internet)."
	@echo ""
