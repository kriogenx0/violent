#include <JuceHeader.h>

int main (int, char**)
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);

    // Only run this project's own tests (category "Processor") — runAllTests()
    // would also pull in JUCE's own internal module test suite, which is slow
    // and not something this project needs to verify on every run.
    runner.runTestsInCategory ("Processor");

    int numFailures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        numFailures += runner.getResult (i)->failures;

    juce::Logger::writeToLog (numFailures == 0
        ? "\nAll tests passed."
        : juce::String (numFailures) + " test failure(s).");

    return numFailures > 0 ? 1 : 0;
}
