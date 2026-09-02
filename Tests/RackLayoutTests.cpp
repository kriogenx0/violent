#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Views/RackView.h"
#include <vector>

// Reproduces "the click mask is off after adding a generator/modulator": the
// rack panels are laid out top-to-bottom by ScalableRackComponent::resized(),
// so adding a card to one panel pushes every panel below it down. These tests
// add cards the same way the "+ Add Generator"/"+ LFO" buttons do — mutating
// processor state then calling the panel's refreshFromState(), exactly what
// GeneratorPanel/ModulatorPanel::onLayoutChanged ultimately triggers via
// RackScaler::updateLayout() — and check that hit-testing (Component::
// getComponentAt(), the same lookup real mouse clicks use) at the boundary
// between two panels tracks the new layout rather than the stale one.
namespace
{

bool hitTestResolvesInside (juce::Component& root, juce::Point<int> localPoint,
                             juce::Component& expectedAncestor)
{
    auto* hit = root.getComponentAt (localPoint);
    return hit != nullptr && (hit == &expectedAncestor || expectedAncestor.isParentOf (hit));
}

void collectSliders (juce::Component& c, std::vector<juce::Slider*>& out)
{
    for (int i = 0; i < c.getNumChildComponents(); ++i)
    {
        auto* child = c.getChildComponent (i);
        if (auto* slider = dynamic_cast<juce::Slider*> (child))
            out.push_back (slider);
        collectSliders (*child, out);
    }
}

// Depth-first collection of every descendant of type T - used to audit every
// button in the rack at once, rather than hand-picking a few boundary points.
template <typename T>
void collectAll (juce::Component& c, std::vector<T*>& out)
{
    for (int i = 0; i < c.getNumChildComponents(); ++i)
    {
        auto* child = c.getChildComponent (i);
        if (auto* t = dynamic_cast<T*> (child))
            out.push_back (t);
        collectAll<T> (*child, out);
    }
}

// isVisible() is only the local flag - a component can report it while an
// ancestor (e.g. a type-specific controls container hidden by showForType())
// is hidden, in which case it isn't actually reachable by a click. Walks up
// to root checking every level, without needing isShowing()'s requirement of
// a real desktop peer.
bool isVisibleUpTo (juce::Component& c, juce::Component& root)
{
    for (juce::Component* p = &c; p != nullptr; p = p->getParentComponent())
    {
        if (! p->isVisible()) return false;
        if (p == &root) break;
    }
    return true;
}

// Mirrors what RackScaler does to the rack: RackScaler::RackScaler() calls
// addAndMakeVisible(rack) (components are invisible, and so un-hit-testable,
// until shown), and updateLayout() sets its bounds from preferredHeight()
// (minus the zoom AffineTransform, which doesn't affect local hit-testing).
void relayout (ScalableRackComponent& rack)
{
    rack.setVisible (true);
    rack.setBounds (0, 0, ScalableRackComponent::BASE_WIDTH, rack.preferredHeight());
}

// Runs pending MessageManager::callAsync() work (e.g. the deferred row/card
// teardown in GeneratorPanel/ModulatorPanel/etc.'s onRemove) without needing
// a real event loop. JUCE_MODAL_LOOPS_PERMITTED is enabled for this test
// target specifically so runDispatchLoopUntil() is available here.
void pumpPendingMessages()
{
    juce::MessageManager::getInstance()->runDispatchLoopUntil (20);
}

// Depth-first search for the first descendant of type T - used to grab a
// real button (e.g. a DeleteButton) to click through the actual JUCE mouse
// pipeline, rather than invoking its callback directly.
template <typename T>
T* findFirstOfType (juce::Component& c)
{
    if (auto* t = dynamic_cast<T*> (&c)) return t;
    for (int i = 0; i < c.getNumChildComponents(); ++i)
        if (auto* found = findFirstOfType<T> (*c.getChildComponent (i)))
            return found;
    return nullptr;
}

// Depth-first search for the first juce::Button descendant with the given
// text - used to click "+ Add Synth"/"+ <MIDI mod type>" for real, since
// GeneratorPanel/MidiModifierPanel don't expose those buttons directly.
juce::Button* findButtonWithText (juce::Component& c, const juce::String& text)
{
    if (auto* b = dynamic_cast<juce::Button*> (&c))
        if (b->getButtonText() == text)
            return b;
    for (int i = 0; i < c.getNumChildComponents(); ++i)
        if (auto* found = findButtonWithText (*c.getChildComponent (i), text))
            return found;
    return nullptr;
}

// Adds host to the desktop so it gets a real ComponentPeer, then sanity-checks
// that this specific console-app test binary is actually capable of
// participating in real, OS-routed hit-testing (see the long comment at the
// first call site for why that can't be assumed). Returns false - having
// already logged why and cleaned up - if not, so callers can skip their
// real-pipeline assertions rather than report a false failure.
bool activateHostForRealMousePipeline (juce::Component& host, juce::UnitTest& test)
{
    host.addToDesktop (0);
    host.setVisible (true);
    host.toFront (true);
    juce::MessageManager::getInstance()->runDispatchLoopUntil (20);

    test.expect (host.getPeer() != nullptr, "host window should have a real ComponentPeer once added to the desktop");
    if (host.getPeer() == nullptr) return false;

    if (! host.contains (host.getLocalBounds().getCentre()))
    {
        test.logMessage ("Skipping real-mouse-pipeline assertions: this console app's "
                    "ComponentPeer::contains() returns false even at its own window's "
                    "centre, so JUCE's real event pipeline (MouseInputSourceImpl::"
                    "findComponentAt(), juce_MouseInputSourceImpl.h:74-88) can never "
                    "resolve a componentUnderMouse in this environment - see "
                    "isWindowAtPoint() in juce_NSViewComponentPeer_mac.mm. Not a rack "
                    "layout bug; this test binary just isn't an activated GUI app.");
        host.removeFromDesktop();
        return false;
    }
    return true;
}

// Sends a synthetic mouseDown then mouseUp at posWithinPeer through the real
// ComponentPeer::handleMouseEvent() entry point - the same one native OS event
// callbacks use - so a real Button click (including its own internal
// pressed/hover state) fires, not just its onClick callback invoked directly.
void realClickAt (juce::ComponentPeer& peer, juce::Point<float> posWithinPeer)
{
    peer.handleMouseEvent (juce::MouseInputSource::InputSourceType::mouse, posWithinPeer,
                            juce::ModifierKeys (juce::ModifierKeys::leftButtonModifier),
                            juce::MouseInputSource::defaultPressure, juce::MouseInputSource::defaultOrientation,
                            juce::Time::currentTimeMillis());
    peer.handleMouseEvent (juce::MouseInputSource::InputSourceType::mouse, posWithinPeer, juce::ModifierKeys(),
                            juce::MouseInputSource::defaultPressure, juce::MouseInputSource::defaultOrientation,
                            juce::Time::currentTimeMillis());
}

class RackLayoutTests : public juce::UnitTest
{
public:
    RackLayoutTests() : juce::UnitTest ("Rack layout", "Views") {}

    void runTest() override
    {
        beginTest ("Adding a second generator moves the generator/effects click boundary down");
        {
            ViolentAudioProcessor proc;
            ScalableRackComponent rack (proc);
            relayout (rack);

            const int x = 10;
            const int oldBoundaryY = rack.getGeneratorPanel().getBottom() + 2;
            expect (hitTestResolvesInside (rack, { x, oldBoundaryY }, rack.getEffectPanel()),
                    "just below the single default generator should hit the effects panel");

            proc.numActiveGenerators = 2;
            rack.getGeneratorPanel().refreshFromState();
            relayout (rack);

            const int newBoundaryY = rack.getGeneratorPanel().getBottom() + 2;
            expect (newBoundaryY > oldBoundaryY, "adding a card should make the generator panel taller");

            expect (hitTestResolvesInside (rack, { x, oldBoundaryY }, rack.getGeneratorPanel()),
                    "the old boundary point is now inside the taller generator panel, not the effects panel");
            expect (hitTestResolvesInside (rack, { x, newBoundaryY }, rack.getEffectPanel()),
                    "the new boundary point should hit the effects panel");
        }

        beginTest ("Adding a modulator moves the modulator/mixer click boundary down");
        {
            ViolentAudioProcessor proc;
            ScalableRackComponent rack (proc);
            relayout (rack);

            const int x = 10;
            const int oldBoundaryY = rack.getModulatorPanel().getBottom() + 2;
            expect (hitTestResolvesInside (rack, { x, oldBoundaryY }, rack.getMixerPanel()),
                    "just below the empty modulator panel should hit the mixer panel");

            proc.numModulators = 1;
            proc.modulators[0].sourceType = ModulatorSourceType::LFO;
            rack.getModulatorPanel().refreshFromState();
            relayout (rack);

            const int newBoundaryY = rack.getModulatorPanel().getBottom() + 2;
            expect (newBoundaryY > oldBoundaryY, "adding a modulator row should make the modulator panel taller");

            expect (hitTestResolvesInside (rack, { x, oldBoundaryY }, rack.getModulatorPanel()),
                    "the old boundary point is now inside the taller modulator panel, not the mixer panel");
            expect (hitTestResolvesInside (rack, { x, newBoundaryY }, rack.getMixerPanel()),
                    "the new boundary point should hit the mixer panel");
        }

        beginTest ("A freshly-added generator card's own bottom-most control is hit-testable, not the panel behind it");
        {
            ViolentAudioProcessor proc;
            ScalableRackComponent rack (proc);
            relayout (rack);

            proc.numActiveGenerators = 2;
            rack.getGeneratorPanel().refreshFromState();
            relayout (rack);

            auto& secondCard = *rack.getGeneratorPanel().getCard (1);
            const juce::Point<int> nearCardBottom (secondCard.getWidth() / 2, secondCard.getHeight() - 4);
            expect (hitTestResolvesInside (secondCard, nearCardBottom, secondCard),
                    "a point near the new card's own bottom edge should resolve to one of its own children");
        }

        beginTest ("Removing a generator via its card's onRemove doesn't destroy the card synchronously (would UAF the button mid-click)");
        {
            // Regression test for "click mask is off after adding then removing a
            // generator": onRemove used to tear the card down (including the
            // DeleteButton invoking it) synchronously, freeing the component while
            // it was still on the call stack inside Button::mouseUp. That undefined
            // behaviour left JUCE's mouse-hover/hit-test tracking pointing at freed
            // memory. The fix defers the teardown via MessageManager::callAsync, so
            // firing onRemove must not change generator count (or free the card)
            // before that async message is dispatched - checked here by asserting
            // the state hasn't moved and the card is still alive right after the
            // callback returns, without pumping the message loop.
            ViolentAudioProcessor proc;
            ScalableRackComponent rack (proc);
            relayout (rack);

            proc.numActiveGenerators = 2;
            rack.getGeneratorPanel().refreshFromState();
            relayout (rack);

            auto* secondCard = rack.getGeneratorPanel().getCard (1);
            expect (secondCard != nullptr && secondCard->onRemove != nullptr,
                    "second card should exist with an onRemove callback");
            secondCard->onRemove();

            expectEquals (proc.numActiveGenerators, 2,
                    "removal must be deferred - state shouldn't change synchronously inside onRemove");
            expect (rack.getGeneratorPanel().getCard (1) == secondCard,
                    "the card must still be alive immediately after onRemove - it mustn't be freed while still on its own click's call stack");

            // Drain the callAsync message posted by onRemove() before proc/rack go
            // out of scope below - otherwise it's left pointing at freed memory and
            // fires during a later test's runDispatchLoopUntil(), crashing there
            // instead of here.
            pumpPendingMessages();
            expectEquals (proc.numActiveGenerators, 1,
                    "the deferred removal should have completed once the message loop ran");
        }

        beginTest ("Cursor hit-testing stays correct through a sequence of adding and removing generators/modulators");
        {
            // Broader regression coverage for "click mask is off after adding then
            // removing components": rather than a single add, this drives several
            // add/remove cycles across two panels (mirroring a user clicking
            // "+ Add Generator"/"+ LFO" and the "x" delete button repeatedly) and
            // checks, after every step, that the point just below each panel's
            // live bottom edge ("the cursor") resolves into the panel that's
            // actually there, not a stale one left over from a previous layout.
            ViolentAudioProcessor proc;
            ScalableRackComponent rack (proc);
            relayout (rack);

            auto& genPanel = rack.getGeneratorPanel();
            auto& modPanel = rack.getModulatorPanel();
            auto& fxPanel  = rack.getEffectPanel();

            const int x = 10;

            auto checkBoundaries = [&] (const juce::String& when)
            {
                relayout (rack);
                expect (hitTestResolvesInside (rack, { x, genPanel.getBottom() + 2 }, fxPanel),
                        "just below the generator panel should hit the effects panel " + when);
                expect (hitTestResolvesInside (rack, { x, fxPanel.getBottom() + 2 }, modPanel),
                        "just below the effects panel should hit the modulator panel " + when);
            };

            checkBoundaries ("initially, with one generator and no modulators");

            // Add generators up to 4, like clicking "+ Add Generator" three times.
            for (int i = 0; i < 3; ++i)
            {
                proc.numActiveGenerators++;
                genPanel.refreshFromState();
                checkBoundaries ("after adding generator #" + juce::String (proc.numActiveGenerators));
            }

            // Add two LFO modulators, like clicking "+ LFO" twice.
            for (int i = 0; i < 2; ++i)
            {
                const int m = proc.numModulators++;
                proc.modulators[(size_t) m].sourceType = ModulatorSourceType::LFO;
                modPanel.refreshFromState();
                checkBoundaries ("after adding modulator #" + juce::String (proc.numModulators));
            }

            // Remove the first generator via its card's real onRemove callback
            // (same path the "x" button uses), then pump the deferred teardown.
            genPanel.getCard (0)->onRemove();
            pumpPendingMessages();
            checkBoundaries ("after removing the first generator");

            // Remove a modulator the same way.
            modPanel.getRow (0)->onRemove();
            pumpPendingMessages();
            checkBoundaries ("after removing the first modulator");

            // Add one more generator on top of the removals, to check the panel
            // recovers correctly rather than compounding any stale state.
            proc.numActiveGenerators++;
            genPanel.refreshFromState();
            checkBoundaries ("after adding another generator following the removals");
        }

        beginTest ("Cursor hit-testing stays correct after adding a synth, adding a MIDI modifier, and deleting an effect");
        {
            // Targeted regression coverage for the specific combo reported as
            // still broken after 2936c94's deferred-teardown fix: add a synth
            // (GeneratorPanel's "+ Add Synth"), add a MIDI modifier
            // (MidiModifierPanel's "+ <type>"), then delete an effect
            // (EffectRow's "x") - each of those mutates a *different* panel,
            // so together they exercise every boundary in the current
            // top-to-bottom order (MIDI Modifiers, Generators, Effects,
            // Modulators, Mixer - see ScalableRackComponent::resized()).
            ViolentAudioProcessor proc;
            ScalableRackComponent rack (proc);
            relayout (rack);

            // Start with one effect already present, so there's something to
            // delete once the additions above it have pushed it down.
            proc.numEffects = 1;
            proc.effects[0].type = FxType::Distortion;
            rack.getEffectPanel().refreshFromState();
            relayout (rack);

            auto& midiPanel = rack.getMidiModifierPanel();
            auto& genPanel  = rack.getGeneratorPanel();
            auto& fxPanel   = rack.getEffectPanel();
            auto& modPanel  = rack.getModulatorPanel();
            auto& mixPanel  = rack.getMixerPanel();

            const int x = 10;

            auto checkBoundaries = [&] (const juce::String& when)
            {
                relayout (rack);
                expect (hitTestResolvesInside (rack, { x, midiPanel.getBottom() + 2 }, genPanel),
                        "just below the MIDI modifier panel should hit the generator panel " + when);
                expect (hitTestResolvesInside (rack, { x, genPanel.getBottom() + 2 }, fxPanel),
                        "just below the generator panel should hit the effects panel " + when);
                expect (hitTestResolvesInside (rack, { x, fxPanel.getBottom() + 2 }, modPanel),
                        "just below the effects panel should hit the modulator panel " + when);
                expect (hitTestResolvesInside (rack, { x, modPanel.getBottom() + 2 }, mixPanel),
                        "just below the modulator panel should hit the mixer panel " + when);
            };

            checkBoundaries ("initially, with one generator, no MIDI modifiers, and one effect");

            // "+ Add Synth": bumps the generator count the same way
            // GeneratorPanel's addSynthBtn does - a fresh generator already
            // defaults to Synth mode.
            proc.numActiveGenerators++;
            genPanel.refreshFromState();
            checkBoundaries ("after adding a synth");

            // "+ Pitch Shift": same effect on layout as clicking any of
            // MidiModifierPanel's per-type add buttons.
            proc.midiModifiers[(size_t) proc.numMidiModifiers].type = MidiModType::PitchShift;
            proc.numMidiModifiers++;
            midiPanel.refreshFromState();
            checkBoundaries ("after adding a MIDI modifier");

            // Delete the effect via its row's real onRemove callback (the
            // "x" button's actual path), then drain the deferred teardown.
            fxPanel.getRow (0)->onRemove();
            expectEquals (proc.numEffects, 1,
                    "removal must be deferred - state shouldn't change synchronously inside onRemove");
            pumpPendingMessages();
            expectEquals (proc.numEffects, 0, "the deferred removal should have completed once the message loop ran");
            checkBoundaries ("after deleting the effect");
        }

        beginTest ("Cursor click routing stays correct through JUCE's real mouse-event pipeline across an add/remove sequence");
        {
            // The tests above ("Cursor hit-testing stays correct...") only ever
            // call Component::getComponentAt() directly - pure current-geometry
            // hit-testing. They can't catch a bug that lives in JUCE's *cached*
            // mouse-pipeline state (MouseInputSourceImpl::componentUnderMouse,
            // tracked in juce_MouseInputSourceImpl.h), which is exactly the kind
            // of state the original click-mask bug (fixed in 2936c94) corrupted.
            //
            // This test drives the real pipeline: a Component is actually
            // addToDesktop()'d (gets a live ComponentPeer), and synthetic events
            // are fed through ComponentPeer::handleMouseEvent() - the same entry
            // point native OS event callbacks use - so MouseInputSourceImpl's
            // internal hover/down tracking is genuinely exercised, not bypassed.
            ViolentAudioProcessor proc;
            ScalableRackComponent rack (proc);
            RackScaler scaler (rack);
            juce::Viewport viewport;
            viewport.setViewedComponent (&scaler, false);
            viewport.setScrollBarsShown (true, false);

            juce::Component host;
            host.setBounds (0, 0, 1000, 800);
            host.addAndMakeVisible (viewport);
            viewport.setBounds (0, 0, 1000, 800);
            scaler.updateLayout (1.0f);

            // Records which leaf component every *real* mouseDown lands on, via
            // JUCE's own dispatch (Component::internalMouseDown), as opposed to
            // a manual getComponentAt() query.
            struct DownRecorder : public juce::MouseListener
            {
                juce::Component* lastDownTarget = nullptr;
                void mouseDown (const juce::MouseEvent& e) override { lastDownTarget = e.eventComponent; }
            } recorder;
            host.addMouseListener (&recorder, true);

            // Sanity-check that this test binary's real ComponentPeer can
            // actually participate in WindowServer-backed hit-testing.
            // Component::contains() on a top-level component ultimately calls
            // ComponentPeer::contains(), which (on macOS - see
            // juce_NSViewComponentPeer_mac.mm's isWindowAtPoint()) asks the OS
            // "is *my* window the one on top at this screen point" via
            // [NSWindow windowNumberAtPoint:belowWindowWithWindowNumber:]. In a
            // bare juce_add_console_app (not a bundled/activated GUI app), that
            // query can come back false unconditionally, even for a point at
            // the exact centre of the window - meaning no real mouse event can
            // ever resolve to a target here, regardless of whether the rack's
            // own layout/removal logic is correct. That's an environment
            // limitation of this specific test binary, not something a
            // click-routing test can meaningfully assert on, so
            // activateHostForRealMousePipeline() skips rather than reports a
            // false failure either way.
            if (activateHostForRealMousePipeline (host, *this))
            {
                auto* peer = host.getPeer();

                // Deliberately does NOT pump the message loop after each event -
                // handleMouseEvent() resolves componentUnderMouse synchronously,
                // and pumping here would let the real OS deliver genuine
                // mouse-moved events from the actual system cursor into this
                // real (on-screen) test window, clobbering the synthetic
                // position we just fed in.
                auto sendMouseEvent = [&] (juce::Point<float> posWithinPeer, juce::ModifierKeys mods)
                {
                    peer->handleMouseEvent (juce::MouseInputSource::InputSourceType::mouse, posWithinPeer, mods,
                                            juce::MouseInputSource::defaultPressure,
                                            juce::MouseInputSource::defaultOrientation,
                                            juce::Time::currentTimeMillis());
                };

                // A fixed point, in the *host window's own coordinates* - i.e. a
                // cursor that never physically moves for the rest of this test,
                // exactly like a user whose mouse sits still while clicking
                // "+ Add Generator"/"x" elsewhere in the rack. It starts hovering
                // over the effects panel.
                auto toHostLocal = [&] (juce::Point<int> screenPoint)
                {
                    return host.getLocalPoint (nullptr, screenPoint).toFloat();
                };

                auto& genPanel = rack.getGeneratorPanel();
                auto& modPanel = rack.getModulatorPanel();
                auto& midiPanel = rack.getMidiModifierPanel();
                auto& fxPanel  = rack.getEffectPanel();

                const juce::Point<float> cursor = toHostLocal (fxPanel.getScreenBounds().getTopLeft() + juce::Point<int> (10, 5));

                // Establish real hover state at the cursor before anything moves,
                // same as a user resting the mouse over the effects panel.
                sendMouseEvent (cursor, {});
                auto* hoverTarget = juce::Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();
                expect (hoverTarget != nullptr && fxPanel.isParentOf (hoverTarget),
                        "initial hover should resolve into the effects panel");

                auto clickAtCursorAndGetTarget = [&] () -> juce::Component*
                {
                    recorder.lastDownTarget = nullptr;
                    sendMouseEvent (cursor, juce::ModifierKeys (juce::ModifierKeys::leftButtonModifier));
                    sendMouseEvent (cursor, {}); // mouse up, so the next event isn't treated as a drag
                    return recorder.lastDownTarget;
                };

                auto liveTargetAtCursor = [&] () -> juce::Component*
                {
                    // What a fresh, non-cached hit-test says is at the cursor right
                    // now - the "ground truth" the real click should agree with.
                    return host.getComponentAt (cursor.toInt());
                };

                auto* initialClickTarget = clickAtCursorAndGetTarget();
                expect (initialClickTarget == &fxPanel || fxPanel.isParentOf (initialClickTarget),
                        "a real click at the cursor should land on the effects panel before anything is added/removed");

                // Now mutate unrelated panels *above* the effects panel in the
                // rack (generators, modulators, MIDI modifiers) - each of these
                // pushes the effects panel down, moving whatever's actually at
                // the fixed cursor point without any synthetic mouse-move being
                // sent in between. Mirrors clicking "+ Add Generator"/"+ LFO"/the
                // "x" delete button repeatedly while the mouse doesn't move.
                proc.numActiveGenerators = 3;
                genPanel.refreshFromState();
                scaler.updateLayout (1.0f);
                pumpPendingMessages();

                proc.numModulators = 2;
                proc.modulators[0].sourceType = ModulatorSourceType::LFO;
                proc.modulators[1].sourceType = ModulatorSourceType::LFO;
                modPanel.refreshFromState();
                scaler.updateLayout (1.0f);
                pumpPendingMessages();

                proc.numMidiModifiers = 1;
                midiPanel.refreshFromState();
                scaler.updateLayout (1.0f);
                pumpPendingMessages();

                genPanel.getCard (0)->onRemove();
                pumpPendingMessages();
                scaler.updateLayout (1.0f);

                proc.numEffects = 1;
                fxPanel.refreshFromState();
                scaler.updateLayout (1.0f);
                pumpPendingMessages();

                // No sendMouseEvent() call has been made since establishing hover
                // above - so componentUnderMouse is exactly as stale as it would
                // be for a real user's stationary cursor.
                auto* liveTarget = liveTargetAtCursor();
                expect (liveTarget != nullptr, "something should now be at the cursor after the add/remove sequence");

                auto* clickTarget = clickAtCursorAndGetTarget();
                expect (clickTarget != nullptr, "the real click should have landed on something");

                expect (clickTarget == liveTarget || (liveTarget != nullptr && liveTarget->isParentOf (clickTarget))
                        || (clickTarget != nullptr && clickTarget->isParentOf (liveTarget)),
                        "a real mouseDown routed through JUCE's actual event pipeline must land on the "
                        "same component the current (post add/remove) layout says is at that point - "
                        "not a stale target left over from before the layout changed");

                host.removeFromDesktop();
            }
        }

        beginTest ("Cursor click routing stays correct through a real click on \"+ Add Synth\", a real click adding a MIDI modifier, and a real click deleting an effect");
        {
            // Same rationale as the test above, but driving the exact combo
            // reported as still broken: clicks on the actual buttons (found
            // by walking the component tree, since GeneratorPanel/
            // MidiModifierPanel/EffectRow keep them private) rather than
            // calling their callbacks directly, so a real Button::mouseUp -
            // the call stack the original click-mask bug (fixed in 2936c94)
            // lived in - is genuinely exercised for all three actions,
            // including the delete.
            ViolentAudioProcessor proc;
            ScalableRackComponent rack (proc);
            RackScaler scaler (rack);
            juce::Viewport viewport;
            viewport.setViewedComponent (&scaler, false);
            viewport.setScrollBarsShown (true, false);

            juce::Component host;
            host.setBounds (0, 0, 1000, 800);
            host.addAndMakeVisible (viewport);
            viewport.setBounds (0, 0, 1000, 800);

            // Start with one effect already present, so there's a real
            // DeleteButton to click later.
            proc.numEffects = 1;
            proc.effects[0].type = FxType::Distortion;
            rack.getEffectPanel().refreshFromState();
            scaler.updateLayout (1.0f);

            if (activateHostForRealMousePipeline (host, *this))
            {
                auto* peer = host.getPeer();

                auto toHostLocal = [&] (juce::Point<int> screenPoint)
                {
                    return host.getLocalPoint (nullptr, screenPoint).toFloat();
                };

                auto& mixPanel = rack.getMixerPanel();

                // A fixed point that never moves for the rest of this test -
                // a user whose mouse rests over the Mixer (below everything
                // that's about to move) while they click the three buttons
                // above it elsewhere on screen.
                const juce::Point<float> stationaryCursor =
                    toHostLocal (mixPanel.getScreenBounds().getTopLeft() + juce::Point<int> (10, 5));

                realClickAt (*peer, stationaryCursor); // establish real hover, same as the test above
                auto* hoverTarget = juce::Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();
                expect (hoverTarget != nullptr && mixPanel.isParentOf (hoverTarget),
                        "initial hover should resolve into the mixer panel");

                // "+ Add Synth": a real click on the actual button, not a
                // direct call to its onClick.
                auto* addSynthBtn = findButtonWithText (rack.getGeneratorPanel(), "+ Add Synth");
                expect (addSynthBtn != nullptr, "generator panel should have a \"+ Add Synth\" button");
                if (addSynthBtn != nullptr)
                {
                    realClickAt (*peer, toHostLocal (addSynthBtn->getScreenBounds().getCentre()));
                    scaler.updateLayout (1.0f);
                }

                // "+ Pitch Shift" (any MIDI modifier type add button): same deal.
                auto* addMidiModBtn = findButtonWithText (rack.getMidiModifierPanel(), "+ Pitch Shift");
                expect (addMidiModBtn != nullptr, "MIDI modifier panel should have a \"+ Pitch Shift\" button");
                if (addMidiModBtn != nullptr)
                {
                    realClickAt (*peer, toHostLocal (addMidiModBtn->getScreenBounds().getCentre()));
                    scaler.updateLayout (1.0f);
                }

                // Delete the effect via a real click on its row's actual
                // DeleteButton - the exact call stack (Button::mouseUp
                // invoking onRemove(), which used to tear the row down
                // synchronously) the original bug lived in.
                auto* effectRow = rack.getEffectPanel().getRow (0);
                auto* deleteBtn = effectRow != nullptr ? findFirstOfType<DeleteButton> (*effectRow) : nullptr;
                expect (deleteBtn != nullptr, "the effect row should have a real DeleteButton to click");
                if (deleteBtn != nullptr)
                {
                    realClickAt (*peer, toHostLocal (deleteBtn->getScreenBounds().getCentre()));
                    pumpPendingMessages(); // drain the deferred removal
                    scaler.updateLayout (1.0f);
                }

                expectEquals (rack.getEffectPanel().getNumRows(), 0,
                        "the effect should actually be gone after the deferred removal runs");

                // No event has been sent at stationaryCursor since the initial
                // hover above - exactly as stale as a real user's unmoved
                // mouse would be after three unrelated clicks elsewhere.
                auto* liveTarget = host.getComponentAt (stationaryCursor.toInt());
                expect (liveTarget != nullptr, "something should be at the stationary cursor after the add/add/delete sequence");

                struct DownRecorder : public juce::MouseListener
                {
                    juce::Component* lastDownTarget = nullptr;
                    void mouseDown (const juce::MouseEvent& e) override { lastDownTarget = e.eventComponent; }
                } recorder;
                host.addMouseListener (&recorder, true);
                realClickAt (*peer, stationaryCursor);
                auto* clickTarget = recorder.lastDownTarget;
                expect (clickTarget != nullptr, "the real click should have landed on something");

                expect (clickTarget == liveTarget || (liveTarget != nullptr && liveTarget->isParentOf (clickTarget))
                        || (clickTarget != nullptr && clickTarget->isParentOf (liveTarget)),
                        "a real mouseDown at the stationary cursor must land on the same component the "
                        "current (post add-synth/add-modifier/delete-effect) layout says is there - not a "
                        "stale target left over from before those three actions");

                host.removeFromDesktop();
            }
        }

        beginTest ("Each generator card's own Gain knob hit-tests correctly, not a different card's, on a fresh multi-generator load at every zoom level");
        {
            // Targeted regression test for a report that a fresh load (no
            // add/remove at all - e.g. restoring a saved standalone session
            // with several generators already present) has generator card 1
            // (and later) hit-testing/hovering at the wrong position, while
            // card 0 is fine. Exercises the *real* transform path a mouse
            // actually goes through - RackScaler's AffineTransform::scale,
            // which every zoom level in the app (0.85x-1.5x, see
            // PluginEditorEditor::setUiScale) applies to the whole rack - by
            // mapping each knob's own bounds into the scaler's coordinate
            // space (the same composition JUCE's real hit-testing performs)
            // rather than assuming no transform is involved.
            ViolentAudioProcessor proc;
            proc.numActiveGenerators = 3;
            ScalableRackComponent rack (proc);
            rack.refreshFromState(); // builds all 3 cards up front, as if restored from saved state
            RackScaler scaler (rack);
            scaler.setVisible (true); // Viewport::setViewedComponent() does this in the real app; getComponentAt() requires it

            for (float scale : { 0.85f, 1.0f, 1.25f, 1.5f })
            {
                scaler.updateLayout (scale);

                std::array<juce::Slider*, 3> gainSliders {};
                for (int g = 0; g < 3; ++g)
                {
                    auto* card = rack.getGeneratorPanel().getCard (g);
                    expect (card != nullptr, "generator " + juce::String (g) + " should have a card");
                    if (card == nullptr) continue;

                    std::vector<juce::Slider*> sliders;
                    collectSliders (*card, sliders);
                    expect (! sliders.empty(), "card " + juce::String (g) + " should contain sliders");
                    if (! sliders.empty())
                        gainSliders[(size_t) g] = sliders.front(); // GeneratorCard adds gainKnob first
                }

                for (int g = 0; g < 3; ++g)
                {
                    auto* slider = gainSliders[(size_t) g];
                    if (slider == nullptr) continue;

                    // The exact point a real mouse cursor would be at, in the
                    // scaler's own (post-zoom) coordinate space - composing
                    // every transform between the slider and the scaler,
                    // same as JUCE's own MouseInputSourceImpl::findComponentAt().
                    const auto pointInScaler = scaler.getLocalArea (slider, slider->getLocalBounds()).getCentre();
                    auto* hit = scaler.getComponentAt (pointInScaler);

                    expect (hit != nullptr && (hit == slider || slider->isParentOf (hit)),
                            "at zoom " + juce::String (scale) + ", generator " + juce::String (g)
                                + "'s own Gain knob position should hit-test to its own knob");

                    for (int other = 0; other < 3; ++other)
                    {
                        if (other == g || gainSliders[(size_t) other] == nullptr) continue;
                        expect (hit != gainSliders[(size_t) other],
                                "at zoom " + juce::String (scale) + ", generator " + juce::String (g)
                                    + "'s Gain knob position must not hit-test to generator " + juce::String (other) + "'s Gain knob");
                    }
                }
            }
        }

        beginTest ("Every button in the rack still hit-tests to itself, wherever it ends up, after adding a synth");
        {
            // Broader, unbiased sweep in the same spirit as the boundary-
            // specific tests above: rather than hand-picking a few points,
            // this audits every single juce::Button in the whole rack (add-
            // type buttons, delete buttons, enable toggles, routing buttons,
            // mode buttons - everything) after a layout-changing action, so
            // a stale/mispositioned hit region anywhere wouldn't need its
            // own dedicated test to be caught.
            ViolentAudioProcessor proc;
            proc.numActiveGenerators = 2;
            proc.numMidiModifiers = 1;
            proc.midiModifiers[0].type = MidiModType::PitchShift;
            proc.numEffects = 1;
            proc.effects[0].type = FxType::Distortion;
            ScalableRackComponent rack (proc);
            rack.refreshFromState();
            relayout (rack);

            std::vector<juce::Button*> buttonsBefore;
            collectAll (rack, buttonsBefore);
            expect (buttonsBefore.size() > 20, "rack should contain many buttons to audit (found "
                    + juce::String ((int) buttonsBefore.size()) + ")");

            // "+ Add Synth": same effect on layout as GeneratorPanel's real button.
            proc.numActiveGenerators++;
            rack.getGeneratorPanel().refreshFromState();
            relayout (rack);

            int checked = 0;
            for (auto* btn : buttonsBefore)
            {
                // Some buttons (e.g. routing buttons for generators that don't
                // exist yet, or type-specific controls hidden by
                // showForType()) are legitimately hidden - a hidden button
                // isn't hit-testable and clicking where it *would* be should
                // reach whatever's actually visible there instead, so skip
                // those. Checked up the ancestor chain (isVisible() alone is
                // only the local flag); isShowing() isn't used since it
                // requires a real desktop peer, which this detached test
                // rack doesn't have.
                if (! isVisibleUpTo (*btn, rack)) continue;

                const auto centreInRack = rack.getLocalArea (btn, btn->getLocalBounds()).getCentre();
                expect (hitTestResolvesInside (rack, centreInRack, *btn),
                        "button \"" + btn->getButtonText() + "\" no longer hit-tests to itself at its "
                        "own current on-screen position after adding a synth");
                ++checked;
            }
            expect (checked > 20, "should have actually audited a meaningful number of visible buttons (audited "
                    + juce::String (checked) + ")");
        }

        beginTest ("A generator card's own bounds-relative-to-its-panel stay fixed when a sibling panel above it changes size");
        {
            // Documents the exact mechanical precondition that makes "mouse
            // position goes stale after an unrelated add/remove elsewhere in
            // the rack" possible at all: JUCE's automatic re-hover correction
            // (Component::sendFakeMouseMove(), fired from inside setBounds() -
            // see juce_Component.cpp) only fires for a component whose OWN
            // setBounds() call detects a real change. When a MIDI modifier is
            // added above the Generators panel, GeneratorPanel's OWN bounds
            // move within the rack (correctly triggering that correction) -
            // but every GeneratorCard's bounds *relative to its own parent
            // panel* are recomputed from that panel's getLocalBounds() every
            // relayout, which is unaffected by where the panel itself now
            // sits in the rack. So each card's own setBounds() call sees the
            // exact same rectangle every time and is a no-op, relying
            // entirely on GeneratorPanel's own move to trigger the global
            // recheck on its behalf.
            //
            // This test does NOT prove that reliance actually fails on real
            // hardware - JUCE's correction is designed to work via exactly
            // this kind of ancestor-triggered global recheck, and confirming
            // whether a real OS mouse event resolves correctly afterwards
            // needs a genuinely foreground-activated window, which no batch-
            // run test process on this machine can obtain. That was checked
            // directly, not assumed: this test target was temporarily
            // rebuilt as a real .app bundle (juce_add_gui_app, the same
            // mechanism JUCE's own Standalone format uses) and launched via
            // `open` exactly like the real app - ComponentPeer::contains()
            // still returned false. So this test exists to guard the specific
            // code shape (and flag it clearly if a future refactor changes
            // it), not to stand in for the coverage that's structurally
            // unreachable here.
            ViolentAudioProcessor proc;
            ScalableRackComponent rack (proc);
            relayout (rack);

            auto* card0 = rack.getGeneratorPanel().getCard (0);
            expect (card0 != nullptr, "generator 0 should have a card");
            if (card0 == nullptr) return;

            const auto card0BoundsBefore = card0->getBounds(); // relative to GeneratorPanel, its immediate parent
            const int genPanelYBefore = rack.getGeneratorPanel().getY(); // relative to the rack

            proc.numMidiModifiers = 1;
            proc.midiModifiers[0].type = MidiModType::PitchShift;
            rack.getMidiModifierPanel().refreshFromState();
            relayout (rack);

            expect (rack.getGeneratorPanel().getY() > genPanelYBefore,
                    "sanity check: adding a MIDI modifier should actually move the generator panel down");
            expect (card0->getBounds() == card0BoundsBefore,
                    "generator 0's card bounds relative to its own panel should be unaffected by the panel's "
                    "own position moving - confirming its own setBounds() call is a no-op here");
        }

        beginTest ("Every ConstrainedKnobSlider's clamped drag margin still reaches the parameter's full range");
        {
            ViolentAudioProcessor proc;
            ScalableRackComponent rack (proc);
            relayout (rack);

            auto& card = *rack.getGeneratorPanel().getCard (0);
            std::vector<juce::Slider*> sliders;
            collectSliders (card, sliders);
            expect (! sliders.empty(), "generator card should contain sliders to check");
            for (auto* slider : sliders)
            {
                // pixelsForFullDragExtent, set by ConstrainedKnobSlider::resized(), must be
                // small enough that the mouseDrag() clamp (knob bounds + a small margin)
                // doesn't leave most of the range unreachable before the drag saturates.
                const int usableTravel = juce::jmin (slider->getWidth(), slider->getHeight()) + 20;
                expectEquals (slider->getMouseDragSensitivity(), juce::jmax (10, usableTravel));
            }
        }

        beginTest ("A real mouseDown+mouseDrag sequence on a knob still changes its value");
        {
            // Checks the actual JUCE mouse-event path (mouseDown() then
            // mouseDrag()), not just the sensitivity math in the test above,
            // so a real interaction regression in ConstrainedKnobSlider or
            // LabelledKnob's chosen SliderStyle wouldn't slip through silently.
            ViolentAudioProcessor proc;
            ScalableRackComponent rack (proc);
            relayout (rack);

            auto& card = *rack.getGeneratorPanel().getCard (0);
            std::vector<juce::Slider*> sliders;
            collectSliders (card, sliders);
            expect (! sliders.empty(), "generator card should contain sliders to check");

            auto* slider = sliders.front();
            const double before = slider->getValue();

            auto source = juce::Desktop::getInstance().getMainMouseSource();
            const auto now = juce::Time::getCurrentTime();
            const auto downPos = slider->getLocalBounds().getCentre().toFloat();
            const auto dragPos = downPos - juce::Point<float> (0.0f, 40.0f); // drag straight up 40px

            juce::MouseEvent down (source, downPos, juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    slider, slider, now, downPos, now, 1, false);
            slider->mouseDown (down);

            juce::MouseEvent drag (source, dragPos, juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    slider, slider, now, downPos, now, 1, true);
            slider->mouseDrag (drag);

            expect (! juce::approximatelyEqual (slider->getValue(), before),
                    "dragging straight up on a knob should change its value");
            expect (slider->getValue() > before,
                    "dragging up should increase the value, not decrease or leave it unchanged");

            juce::MouseEvent up (source, dragPos, juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                  slider, slider, now, downPos, now, 1, true);
            slider->mouseUp (up); // closes the SliderAttachment's change gesture cleanly
        }
    }
};

static RackLayoutTests rackLayoutTests;

} // namespace
