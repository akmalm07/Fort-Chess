-- workspace "Client-Server-Lib"
--     architecture "x64"
--     startproject "Client"

--     configurations { "Debug", "Release" }

-- outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- -- Include directories relative to root folder
-- IncludeDir = {}
-- IncludeDir["ASIO"] = "../vendor/ASIO/include" --asio is a header only library

-- group "Client"
--     include "Client/Client.lua"

-- group "Server"
--     include "Server/Server.lua"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder
IncludeDir = {}
IncludeDir["ASIO"] = "vendor/ASIO/include"

workspace "ClientProject"
    architecture "x64"
    startproject "Client"
    configurations { "Debug", "Release" }


project "Client"
    kind "ConsoleApp" 
    language "C++"
    cppdialect "C++latest"
    staticruntime "on"

    targetdir ("Client/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("Client/bin-int/" .. outputdir .. "/%{prj.name}")

    includedirs 
    {
        "%{IncludeDir.ASIO}",
        "Client/global",            
        "Client/include",           
        "Client/src"
    }

    files 
    {
        "Client/src/**.cpp",           
        "Client/include/**.h",           
        "Client/include/**.inl", 
        "Client/global/**.h",
        "Client/global/**.cpp"
    }

    defines "ASIO_STANDALONE"

    pchheader "headers.h"
    pchsource "Client/global/headers.cpp"

    flags { "Verbose" }

    filter "toolset:msc"
        toolset "msc-v143"
        buildoptions { "/std:c++23" } 
        
    filter "toolset:gcc or toolset:clang"
        buildoptions { "-std=c++23" }

    filter "configurations:Debug"
        defines "DEBUG"
        symbols "On"
        optimize "Off"
        runtime "Release"  

    filter "configurations:Release"
        symbols "Off"
        optimize "On"
        defines "NDEBUG"
        runtime "Release"  

    filter "system:windows"
        systemversion "latest"
        defines "PLATFORM_WINDOWS"
    
    filter "action:vs*"
        defines "_CRT_SECURE_NO_WARNINGS"
        staticruntime "on"

    filter "system:linux or toolset:gcc or toolset:clang"
        buildoptions { "-include pch.h" }

    filter "files:global/headers.cpp"   
        buildoptions { "/Ycheaders.h" }


workspace "ServerProject"
    architecture "x64"
    startproject "Server"
    configurations { "Debug", "Release" }

project "Server"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    includedirs 
    {
        "%{IncludeDir.ASIO}",
        "Server/global",            
        "Server/include",           
        "Server/src"
    }

    files 
    {
        "Server/src/**.cpp",           
        "Server/include/**.h",           
        "Server/include/**.inl", 
        "Server/global/**.h",
        "Server/global/**.cpp"
    }

    defines "ASIO_STANDALONE"

    pchheader "headers.h"
    pchsource "Server/global/headers.cpp"

    flags { "Verbose" }

    filter "toolset:msc"
        toolset "msc-v143"
        buildoptions { "/std:c++23" } 
        
    filter "toolset:gcc or toolset:clang"
        buildoptions { "-std=c++23" }

    filter "configurations:Debug"
        defines "DEBUG"
        symbols "On"
        optimize "Off"
        runtime "Release"  

    filter "configurations:Release"
        symbols "Off"
        optimize "On"
        defines "NDEBUG"
        runtime "Release"  

    filter "system:windows"
        systemversion "latest"
        defines "PLATFORM_WINDOWS"
    
    filter "action:vs*"
        defines "_CRT_SECURE_NO_WARNINGS"
        staticruntime "on"

    filter "system:linux or toolset:gcc or toolset:clang"
        buildoptions { "-include pch.h" }

    filter "files:Server/global/headers.cpp"   
        buildoptions { "/Ycheaders.h" }
