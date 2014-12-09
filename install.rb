#!/usr/bin/env ruby -wKU
# encoding: utf-8

glibdir = "."
Dir.chdir glibdir
glibdir = Dir.pwd

projectNameParts = glibdir.split('/')
projectName = projectNameParts.last;
ENV['JAMOMAPROJECT'] = projectName

puts "Clear usr/local/jamoma folder"

# Clean /usr/local/jamoma folder
`rm /usr/local/jamoma/extensions/*.*`
`rm /usr/local/jamoma/includes/*.*`
`rm /usr/local/jamoma/lib/*.*`

puts "Clear usr/local/lib aliases"
`rm /usr/local/lib/*.ttdylib`
`rm /usr/local/lib/Jamoma*.dylib`

# Prepare directories
FileUtils.mkdir_p("/usr/local/jamoma/extensions") unless File.exist?("/usr/local/jamoma/extensions")
FileUtils.mkdir_p("/usr/local/jamoma/includes") unless File.exist?("/usr/local/jamoma/includes")
FileUtils.mkdir_p("/usr/local/jamoma/lib") unless File.exist?("/usr/local/jamoma/lib")

puts "Copy jamoma includes, libraries and extensions into usr/local/jamoma folder"
Dir.chdir "#{glibdir}"

# Foundation extensions, includes and lib
`cp "#{glibdir}"/../Foundation/extensions/DataspaceLib/build/DataspaceLib.ttdylib /usr/local/jamoma/extensions`
`cp "#{glibdir}"/../Foundation/extensions/NetworkLib/build/NetworkLib.ttdylib /usr/local/jamoma/extensions`

`cp "#{glibdir}"/../Foundation/library/includes/* /usr/local/jamoma/includes`

`cp "#{glibdir}"/../Foundation/library/build/JamomaFoundation.dylib /usr/local/jamoma/lib`

# DSP extensions, includes and lib
`cp "#{glibdir}"/../DSP/extensions/FunctionLib/build/FunctionLib.ttdylib /usr/local/jamoma/extensions`
`cp "#{glibdir}"/../DSP/extensions/AnalysisLib/build/AnalysisLib.ttdylib /usr/local/jamoma/extensions`

`cp "#{glibdir}"/../DSP/library/includes/* /usr/local/jamoma/includes`

`cp "#{glibdir}"/../DSP/library/build/JamomaDSP.dylib /usr/local/jamoma/lib`

# Modular extensions, includes and lib
`cp "#{glibdir}"/../Modular/extensions/MIDI/build/MIDI.ttdylib /usr/local/jamoma/extensions`
`cp "#{glibdir}"/../Modular/extensions/Minuit/build/Minuit.ttdylib /usr/local/jamoma/extensions`
`cp "#{glibdir}"/../Modular/extensions/OSC/build/OSC.ttdylib /usr/local/jamoma/extensions`
`cp "#{glibdir}"/../Modular/extensions/System/build/System.ttdylib /usr/local/jamoma/extensions`

`cp "#{glibdir}"/../Modular/library/includes/TTModular.h /usr/local/jamoma/includes`
`cp "#{glibdir}"/../Modular/library/includes/TTModularIncludes.h /usr/local/jamoma/includes`
`cp "#{glibdir}"/../Modular/library/PeerObject/*.h /usr/local/jamoma/includes`
`cp "#{glibdir}"/../Modular/library/ProtocolLib/Protocol.h /usr/local/jamoma/includes`
`cp "#{glibdir}"/../Modular/library/SchedulerLib/Scheduler.h /usr/local/jamoma/includes`

`cp "#{glibdir}"/../Modular/library/build/JamomaModular.dylib /usr/local/jamoma/lib`

puts "Copy score includes, library and extensions into usr/local/jamoma folder"
Dir.chdir "#{glibdir}"

# Copy Score headers
`cp -f -p ./library/includes/*.h /usr/local/jamoma/includes`
`cp -f -p ./library/tests/*.h /usr/local/jamoma/includes`
`cp -f -p ./extensions/TimePluginLib.h /usr/local/jamoma/includes`

# Copy Score dylibs
`cp -f -p ./library/build/JamomaScore.dylib /usr/local/jamoma/lib`
`cp -f -p ./extensions/Interval/build/Interval.ttdylib /usr/local/jamoma/extensions`
`cp -f -p ./extensions/Automation/build/Automation.ttdylib /usr/local/jamoma/extensions`
`cp -f -p ./extensions/Scenario/build/Scenario.ttdylib /usr/local/jamoma/extensions`

puts "Create alias into usr/local/lib folder"

# Create aliases
`ln -s /usr/local/jamoma/lib/JamomaFoundation.dylib /usr/local/lib/JamomaFoundation.dylib`
`ln -s /usr/local/jamoma/lib/JamomaDSP.dylib /usr/local/lib/JamomaDSP.dylib`
`ln -s /usr/local/jamoma/lib/JamomaModular.dylib /usr/local/lib/JamomaModular.dylib`

`ln -s /usr/local/jamoma/extensions/AnalysisLib.ttdylib /usr/local/lib/AnalysisLib.ttdylib`
`ln -s /usr/local/jamoma/extensions/FunctionLib.ttdylib /usr/local/lib/FunctionLib.ttdylib`
`ln -s /usr/local/jamoma/extensions/NetworkLib.ttdylib /usr/local/lib/NetworkLib.ttdylib`
`ln -s /usr/local/jamoma/extensions/System.ttdylib /usr/local/lib/System.ttdylib`
`ln -s /usr/local/jamoma/extensions/DataspaceLib.ttdylib /usr/local/lib/DataspaceLib.ttdylib`
`ln -s /usr/local/jamoma/extensions/MIDI.ttdylib /usr/local/lib/MIDI.ttdylib`
`ln -s /usr/local/jamoma/extensions/Minuit.ttdylib /usr/local/lib/Minuit.ttdylib`
`ln -s /usr/local/jamoma/extensions/OSC.ttdylib /usr/local/lib/OSC.ttdylib`

`ln -s /usr/local/jamoma/lib/JamomaScore.dylib /usr/local/lib/JamomaScore.dylib`
`ln -s /usr/local/jamoma/extensions/Automation.ttdylib /usr/local/lib/Automation.ttdylib`
`ln -s /usr/local/jamoma/extensions/Interval.ttdylib /usr/local/lib/Interval.ttdylib`
`ln -s /usr/local/jamoma/extensions/Loop.ttdylib /usr/local/lib/Loop.ttdylib`
`ln -s /usr/local/jamoma/extensions/Scenario.ttdylib /usr/local/lib/Scenario.ttdylib`

# Clean includes .orig files
`rm /usr/local/jamoma/includes/*.orig`

puts "done"
puts

