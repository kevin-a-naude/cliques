-- premake5.lua
workspace "DiscreteMaths"
   configurations { "Debug", "Release", "Release-Symbols" }
   platforms { "Win64", "Win32" }
   location (_ACTION)

filter "configurations:Debug*"
   defines { "DEBUG" }
   flags { "Symbols" }

filter "configurations:Release*"
   defines { "NDEBUG" }
   optimize "On"

filter "configurations:Release-Symbols*"
   flags { "Symbols" }

filter { "platforms:Win64" }
   system "Windows"
   architecture "x64"

filter { "platforms:Win32" }
   system "Windows"
   architecture "x86"

project "DiscreteMathLib"
   kind "StaticLib"
   language "C"
   targetdir ("lib/%{cfg.platform}/%{cfg.buildcfg}")

   flags { "EnableSSE", "EnableSSE2" }

   files {
      "src/*.h",
      "src/arena.c",
      "src/bitmanip.c",
      "src/dimacs.c",
      "src/fatal.c",
      "src/platform.c"
   }

project "degseq"
   kind "ConsoleApp"
   language "C"
   targetdir ("bin")
   debugdir "./"
   libdirs {
      "../lib/%{cfg.platform}/%{cfg.buildcfg}"
   }
   includedirs {
      "src/"
   }
   links {
      "DiscreteMathLib"
   }
   flags { "EnableSSE", "EnableSSE2" }

   files {
      "src/graph-gen/*.h",
      "src/graph-gen/degseq.c"
   }

project "gcut"
   kind "ConsoleApp"
   language "C"
   targetdir ("bin")
   debugdir "./"
   libdirs {
      "../lib/%{cfg.platform}/%{cfg.buildcfg}"
   }
   includedirs {
      "src/"
   }
   links {
      "DiscreteMathLib"
   }
   flags { "EnableSSE", "EnableSSE2" }

   files {
      "src/graph-gen/*.h",
      "src/graph-gen/gcut.c"
   }

project "gk"
   kind "ConsoleApp"
   language "C"
   targetdir ("bin")
   debugdir "./"
   libdirs {
      "../lib/%{cfg.platform}/%{cfg.buildcfg}"
   }
   includedirs {
      "src/"
   }
   links {
      "DiscreteMathLib"
   }
   flags { "EnableSSE", "EnableSSE2" }

   files {
      "src/graph-gen/*.h",
      "src/graph-gen/gk.c"
   }

project "gkbip"
   kind "ConsoleApp"
   language "C"
   targetdir ("bin")
   debugdir "./"
   libdirs {
      "../lib/%{cfg.platform}/%{cfg.buildcfg}"
   }
   includedirs {
      "src/"
   }
   links {
      "DiscreteMathLib"
   }
   flags { "EnableSSE", "EnableSSE2" }

   files {
      "src/graph-gen/*.h",
      "src/graph-gen/gkbip.c"
   }

project "gnd"
   kind "ConsoleApp"
   language "C"
   targetdir ("bin")
   debugdir "./"
   libdirs {
      "../lib/%{cfg.platform}/%{cfg.buildcfg}"
   }
   includedirs {
      "src/"
   }
   links {
      "DiscreteMathLib"
   }
   flags { "EnableSSE", "EnableSSE2" }

   files {
      "src/graph-gen/*.h",
      "src/graph-gen/gnd.c"
   }

project "gnm"
   kind "ConsoleApp"
   language "C"
   targetdir ("bin")
   debugdir "./"
   libdirs {
      "../lib/%{cfg.platform}/%{cfg.buildcfg}"
   }
   includedirs {
      "src/"
   }
   links {
      "DiscreteMathLib"
   }
   flags { "EnableSSE", "EnableSSE2" }

   files {
      "src/graph-gen/*.h",
      "src/graph-gen/gnm.c"
   }

project "gnp"
   kind "ConsoleApp"
   language "C"
   targetdir ("bin")
   debugdir "./"
   libdirs {
      "../lib/%{cfg.platform}/%{cfg.buildcfg}"
   }
   includedirs {
      "src/"
   }
   links {
      "DiscreteMathLib"
   }
   flags { "EnableSSE", "EnableSSE2" }

   files {
      "src/graph-gen/*.h",
      "src/graph-gen/gnp.c"
   }

project "tomita"
   kind "ConsoleApp"
   language "C"
   targetdir ("bin")
   debugdir "./"
   libdirs {
      "../lib/%{cfg.platform}/%{cfg.buildcfg}"
   }
   includedirs {
      "src/"
   }
   links {
      "DiscreteMathLib"
   }
   flags { "EnableSSE", "EnableSSE2" }

   files {
      "src/clique-enum/*.h",
      "src/clique-enum/benchmark.c",
      "src/clique-enum/clique_enum.c",
      "src/clique-enum/clique_tomita.c",
      "src/clique-enum/tomita.c"
   }

project "naude"
   kind "ConsoleApp"
   language "C"
   targetdir ("bin")
   debugdir "./"
   libdirs {
      "../lib/%{cfg.platform}/%{cfg.buildcfg}"
   }
   includedirs {
      "src/"
   }
   links {
      "DiscreteMathLib"
   }
   flags { "EnableSSE", "EnableSSE2" }

   files {
      "src/clique-enum/*.h",
      "src/clique-enum/benchmark.c",
      "src/clique-enum/clique_enum.c",
      "src/clique-enum/clique_naude.c",
      "src/clique-enum/naude.c"
   }
