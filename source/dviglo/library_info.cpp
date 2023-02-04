// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "precompiled.h"

#include "library_info.h"

namespace Urho3D
{

const char* GetCompilerDefines()
{
    return ""
#ifdef URHO3D_OPENGL
    "#define URHO3D_OPENGL\n"
#elif defined(URHO3D_D3D11)
    "#define URHO3D_D3D11\n"
#endif
#ifdef URHO3D_SSE
    "#define URHO3D_SSE\n"
#endif
#ifdef URHO3D_DATABASE_ODBC
    "#define URHO3D_DATABASE_ODBC\n"
#elif defined(URHO3D_DATABASE_SQLITE)
    "#define URHO3D_DATABASE_SQLITE\n"
#endif
#ifdef URHO3D_LUAJIT
    "#define URHO3D_LUAJIT\n"
#endif
#ifdef URHO3D_TESTING
    "#define URHO3D_TESTING\n"
#endif
    ;
}

}