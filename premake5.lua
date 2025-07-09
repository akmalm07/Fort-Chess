workspace "Client-Server-Lib"
    architecture "x64"
    startproject "Client"

    configurations { "Debug", "Release" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder
IncludeDir = {}
IncludeDir["ASIO"] = "../vendor/ASIO/include" --asio is a header only library

group "Client"
    include "Client/Client.lua"

group "Server"
    include "Server/Server.lua"