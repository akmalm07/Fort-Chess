outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder
IncludeDir = {}
IncludeDir["ASIO"] = "../vendors/asio/include"
IncludeDir["RAYLIB"] = "../vendors/raylib/include"

LibDir = {}
LibDir["RAYLIB"] = "../vendors/raylib/lib"


workspace "Fort-Chess"
    architecture "x64"
    startproject "fort-chess"
    configurations { "Debug", "Release" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "fort-chess"
    include "client/fort-chess.lua"
-- Include directories relative to root folder

