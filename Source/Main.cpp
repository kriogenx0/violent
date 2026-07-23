// Custom standalone app entry point.
//
// JUCE's default StandaloneFilterApp/StandaloneFilterWindow always draws its
// own JUCE-style title bar (options button top-left, minimise/close top-right).
// We want a real macOS window with native traffic-light controls instead, so
// this file replaces that default (enabled via JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP
// in CMakeLists.txt) and just turns on the native title bar.
#include <juce_core/system/juce_TargetPlatform.h>

#if JucePlugin_Build_Standalone

#include <juce_audio_plugin_client/detail/juce_CheckSettingMacros.h>
#include <juce_audio_plugin_client/detail/juce_IncludeSystemHeaders.h>
#include <juce_audio_plugin_client/detail/juce_IncludeModuleHeaders.h>
#include <juce_audio_plugin_client/detail/juce_PluginUtilities.h>

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

namespace
{
    // With a native title bar, JUCE's own in-content title strip (and the
    // "Options" button that lives in it) collapses to zero height, so we
    // relocate that button into our own header row instead of losing access
    // to the audio/MIDI settings dialog.
    class ViolentStandaloneWindow : public juce::StandaloneFilterWindow
    {
    public:
        using juce::StandaloneFilterWindow::StandaloneFilterWindow;

        void resized() override
        {
            juce::StandaloneFilterWindow::resized();

            for (auto* child : getChildren())
                if (auto* btn = dynamic_cast<juce::TextButton*> (child))
                    if (btn->getButtonText() == "Options")
                        btn->setBounds (150, 13, 70, 24);
        }
    };

    class ViolentStandaloneApp : public juce::JUCEApplication
    {
    public:
        ViolentStandaloneApp()
        {
            juce::PropertiesFile::Options options;
            options.applicationName     = appName;
            options.filenameSuffix      = ".settings";
            options.osxLibrarySubFolder = "Application Support";
            options.folderName          = "";

            appProperties.setStorageParameters (options);
        }

        const juce::String getApplicationName() override           { return appName; }
        const juce::String getApplicationVersion() override        { return JucePlugin_VersionString; }
        bool moreThanOneInstanceAllowed() override                 { return true; }
        void anotherInstanceStarted (const juce::String&) override {}

        void initialise (const juce::String&) override
        {
            mainWindow = std::make_unique<ViolentStandaloneWindow> (
                getApplicationName(),
                juce::LookAndFeel::getDefaultLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId),
                appProperties.getUserSettings(),
                false);

            mainWindow->setUsingNativeTitleBar (true);
            mainWindow->setVisible (true);
        }

        void shutdown() override
        {
            mainWindow = nullptr;
            appProperties.saveIfNeeded();
        }

        void systemRequestedQuit() override
        {
            if (mainWindow != nullptr)
                mainWindow->pluginHolder->savePluginState();

            if (juce::ModalComponentManager::getInstance()->cancelAllModalComponents())
            {
                juce::Timer::callAfterDelay (100, []()
                {
                    if (auto* app = juce::JUCEApplicationBase::getInstance())
                        app->systemRequestedQuit();
                });
            }
            else
            {
                quit();
            }
        }

    private:
        juce::ApplicationProperties appProperties;
        std::unique_ptr<ViolentStandaloneWindow> mainWindow;
        const juce::String appName { juce::CharPointer_UTF8 (JucePlugin_Name) };
    };
}

juce::JUCEApplicationBase* juce_CreateApplication() { return new ViolentStandaloneApp(); }

#endif // JucePlugin_Build_Standalone
