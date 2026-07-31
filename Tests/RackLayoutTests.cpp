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
