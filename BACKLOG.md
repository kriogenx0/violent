# Backlog

Open items as of 2026-10-10. Nothing here blocks anything.

## Check by hand
- **Real pointer.** The "controls drift from their click areas" fix (1f971e7) was verified by driving the
  real standalone window with synthetic events and comparing its pixels, not with a physical
  mouse/trackpad. Worth a manual try: add a few components, click the buttons below them.
- **Zoom on a narrow display.** At 150% the window is 1572px wide. On a display narrower than that the OS
  clamps the width; JUCE now adopts the real size (no more drift), but the rack is wider than the window, so
  its right edge should clip with no horizontal scrollbar (reasoned from the code, not observed - the
  1920/2560-wide displays available while fixing never hit it). Consider capping zoom to what fits.

## Build / tooling
- **`build/` is configured with `-DFETCHCONTENT_SOURCE_DIR_JUCE=build/_deps/juce-src`** (repaired on
  2026-10-10 after the project moved from ~/Sites/Violent, which had made the old cache unusable). That
  override makes CMake ignore `GIT_TAG` in CMakeLists.txt, and `build/_deps/juce-src` is the only copy of
  JUCE on the machine when offline. With network, `rm -rf build && make dev` gives a normal FetchContent
  setup again - do that before bumping JUCE next. Without network, don't `make clean`.
- **JUCE 9.0.0** (released 2026-07-21 alongside 8.0.15) removes `Drawable`'s `Component` base and reworks SVG
  loading; see its `BREAKING_CHANGES.md`. A separate migration; the project is pinned to 8.0.15, the newest
  8.x as of the local clone (fetched 2026-07-31).
- **`ViolentTests` build warning.** `JUCE_STANDALONE_APPLICATION` is defined twice for that target (`=1` by
  `juce_add_console_app`, `=JucePlugin_Build_Standalone` inherited from the plugin target): 'macro redefined'
  warnings from JUCE's own CMake helpers. Harmless; left alone.
