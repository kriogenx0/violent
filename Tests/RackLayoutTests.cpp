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

class RackLayoutTests : public juce::UnitTest
{
public:
    RackLayoutTests() : juce::UnitTest ("Rack layout", "Views") {}

    void runTest() override
    {
        beginTest ("Adding a second generator moves the generator/modulator click boundary down");
        {
            ViolentAudioProcessor proc;
            ScalableRackComponent rack (proc);
            relayout (rack);

            const int x = 10;
            const int oldBoundaryY = rack.getGeneratorPanel().getBottom() + 2;
            expect (hitTestResolvesInside (rack, { x, oldBoundaryY }, rack.getModulatorPanel()),
                    "just below the single default generator should hit the modulator panel");

            proc.numActiveGenerators = 2;
            rack.getGeneratorPanel().refreshFromState();
            relayout (rack);

            const int newBoundaryY = rack.getGeneratorPanel().getBottom() + 2;
            expect (newBoundaryY > oldBoundaryY, "adding a card should make the generator panel taller");

            expect (hitTestResolvesInside (rack, { x, oldBoundaryY }, rack.getGeneratorPanel()),
                    "the old boundary point is now inside the taller generator panel, not the modulator panel");
            expect (hitTestResolvesInside (rack, { x, newBoundaryY }, rack.getModulatorPanel()),
                    "the new boundary point should hit the modulator panel");
        }

        beginTest ("Adding a modulator moves the modulator/effects click boundary down");
        {
            ViolentAudioProcessor proc;
            ScalableRackComponent rack (proc);
            relayout (rack);

            const int x = 10;
            const int oldBoundaryY = rack.getModulatorPanel().getBottom() + 2;
            expect (hitTestResolvesInside (rack, { x, oldBoundaryY }, rack.getEffectPanel()),
                    "just below the empty modulator panel should hit the effects panel");

            proc.numModulators = 1;
            proc.modulators[0].sourceType = ModulatorSourceType::LFO;
            rack.getModulatorPanel().refreshFromState();
            relayout (rack);

            const int newBoundaryY = rack.getModulatorPanel().getBottom() + 2;
            expect (newBoundaryY > oldBoundaryY, "adding a modulator row should make the modulator panel taller");

            expect (hitTestResolvesInside (rack, { x, oldBoundaryY }, rack.getModulatorPanel()),
                    "the old boundary point is now inside the taller modulator panel, not the effects panel");
            expect (hitTestResolvesInside (rack, { x, newBoundaryY }, rack.getEffectPanel()),
                    "the new boundary point should hit the effects panel");
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
                expect (hitTestResolvesInside (rack, { x, genPanel.getBottom() + 2 }, modPanel),
                        "just below the generator panel should hit the modulator panel " + when);
                expect (hitTestResolvesInside (rack, { x, modPanel.getBottom() + 2 }, fxPanel),
                        "just below the modulator panel should hit the effects panel " + when);
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
    }
};

static RackLayoutTests rackLayoutTests;

} // namespace
