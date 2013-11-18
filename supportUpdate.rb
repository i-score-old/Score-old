#!/usr/bin/env ruby -wKU
# encoding: utf-8
puts "update support folder"
glibdir = "."
Dir.chdir glibdir
glibdir = Dir.pwd
Dir.chdir "#{glibdir}"

# Foundation extensions, includes and lib
`cp "#{glibdir}"/../Foundation/extensions/DataspaceLib/build/DataspaceLib.ttdylib "#{glibdir}"/support/jamoma/extensions`
`cp "#{glibdir}"/../Foundation/extensions/NetworkLib/build/NetworkLib.ttdylib "#{glibdir}"/support/jamoma/extensions`

`cp "#{glibdir}"/../Foundation/library/includes/* "#{glibdir}"/support/jamoma/includes`

`cp "#{glibdir}"/../Foundation/library/build/JamomaFoundation.dylib "#{glibdir}"/support/jamoma/lib`

# DSP extensions, includes and lib
`cp "#{glibdir}"/../DSP/extensions/FunctionLib/build/FunctionLib.ttdylib "#{glibdir}"/support/jamoma/extensions`
`cp "#{glibdir}"/../DSP/extensions/AnalysisLib/build/AnalysisLib.ttdylib "#{glibdir}"/support/jamoma/extensions`

`cp "#{glibdir}"/../DSP/library/includes/* "#{glibdir}"/support/jamoma/includes`

`cp "#{glibdir}"/../DSP/library/build/JamomaDSP.dylib "#{glibdir}"/support/jamoma/lib`

# Modular extensions, includes and lib
`cp "#{glibdir}"/../Modular/extensions/Minuit/build/Minuit.ttdylib "#{glibdir}"/support/jamoma/extensions`
`cp "#{glibdir}"/../Modular/extensions/OSC/build/OSC.ttdylib "#{glibdir}"/support/jamoma/extensions`
`cp "#{glibdir}"/../Modular/extensions/System/build/System.ttdylib "#{glibdir}"/support/jamoma/extensions`

`cp "#{glibdir}"/../Modular/library/includes/TTModular.h "#{glibdir}"/support/jamoma/includes`
`cp "#{glibdir}"/../Modular/library/includes/TTModularSymbolCache.h "#{glibdir}"/support/jamoma/includes`
`cp "#{glibdir}"/../Modular/library/PeerObject/*.h "#{glibdir}"/support/jamoma/includes`
`cp "#{glibdir}"/../Modular/library/ProtocolLib/Protocol.h "#{glibdir}"/support/jamoma/includes`
`cp "#{glibdir}"/../Modular/library/SchedulerLib/Scheduler.h "#{glibdir}"/support/jamoma/includes`

`cp "#{glibdir}"/../Modular/library/build/JamomaModular.dylib "#{glibdir}"/support/jamoma/lib`

puts "done"
puts
