// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "application.h"
#include "../io/io_events.h"
#include "../io/log.h"

#include "../core/sdl_helper.h"

#include "../common/debug_new.h"

using namespace std;

namespace dviglo
{

Application::Application() :
    exitCode_(EXIT_SUCCESS)
{
    engineParameters_ = Engine::parse_parameters(GetArguments());

    // Указатели на объекты хранятся в самих классах
    new Log();
    new SdlHelper();
    new Engine(); // Create the Engine, but do not initialize it yet. Subsystems except Graphics & Renderer are registered at this point

    // Subscribe to log messages so that can show errors if ErrorExit() is called with empty message
    log_message.connect(DV_LOG->log_message, bind(&Application::handle_log_message, this, placeholders::_1, placeholders::_2));
}

Application::~Application()
{
    delete Engine::instance_;
    delete SdlHelper::instance_;
    delete Log::instance();
}

int Application::Run()
{
#if !defined(__GNUC__) || __EXCEPTIONS
    try
    {
#endif
        Setup();
        if (exitCode_)
            return exitCode_;

        if (!DV_ENGINE->Initialize(engineParameters_))
        {
            ErrorExit();
            return exitCode_;
        }

        Start();
        if (exitCode_)
            return exitCode_;

        while (!DV_ENGINE->IsExiting())
            DV_ENGINE->run_frame();

        Stop();

        return exitCode_;
#if !defined(__GNUC__) || __EXCEPTIONS
    }
    catch (bad_alloc&)
    {
        ErrorDialog(GetTypeName(), "An out-of-memory error occurred. The application will now exit.");
        return EXIT_FAILURE;
    }
#endif
}

void Application::ErrorExit(const String& message)
{
    DV_ENGINE->Exit(); // Close the rendering window
    exitCode_ = EXIT_FAILURE;

    if (!message.Length())
    {
        ErrorDialog(GetTypeName(), startupErrors_.Length() ? startupErrors_ :
            "Application has been terminated due to unexpected error.");
    }
    else
        ErrorDialog(GetTypeName(), message);
}

void Application::handle_log_message(const String& message, i32 level)
{
    if (level == LOG_ERROR)
    {
        // Strip the timestamp if necessary
        String error = message;
        i32 bracketPos = error.Find(']');
        if (bracketPos != String::NPOS)
            error = error.Substring(bracketPos + 2);

        startupErrors_ += error + "\n";
    }
}

} // namespace dviglo
