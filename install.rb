#!/usr/bin/env ruby -wKU
# encoding: utf-8

glibdir = "."
Dir.chdir glibdir
glibdir = Dir.pwd

projectNameParts = glibdir.split('/')
projectName = projectNameParts.last;
ENV['JAMOMAPROJECT'] = projectName

if  win?
    
elsif mac?

    puts "Clear usr/local/jamoma folder"
    
    # Clean /usr/local/jamoma folder
    `rm /usr/local/jamoma/extensions/*.*`
    `rm /usr/local/jamoma/includes/*.*`
    `rm /usr/local/jamoma/lib/*.*`
    `rm /usr/local/lib/*.ttdylib`
    `rm /usr/local/lib/Jamoma*.dylib`
    
    # Prepare directories
    FileUtils.mkdir_p("/usr/local/jamoma/extensions") unless File.exist?("/usr/local/jamoma/extensions")
    FileUtils.mkdir_p("/usr/local/jamoma/includes") unless File.exist?("/usr/local/jamoma/includes")
    FileUtils.mkdir_p("/usr/local/jamoma/lib") unless File.exist?("/usr/local/jamoma/lib")
    
    puts "Copy support/jamoma folder into usr/local/jamoma folder"
    Dir.chdir "#{glibdir}/support"
    
    # Copy support/jamoma folder into /usr/local/jamoma folder
    `cp -f -p ./jamoma/extensions/* /usr/local/jamoma/extensions`
    `cp -f -p ./jamoma/includes/* /usr/local/jamoma/includes`
    `cp -f -p ./jamoma/lib/* /usr/local/jamoma/lib`
    
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

    `ln -s /usr/local/jamoma/lib/JamomaScore.dylib /usr/local/lib/JamomaScore.dylib`
    `ln -s /usr/local/jamoma/extensions/Automation.ttdylib /usr/local/lib/Automation.ttdylib`
    `ln -s /usr/local/jamoma/extensions/Interval.ttdylib /usr/local/lib/Interval.ttdylib`
    `ln -s /usr/local/jamoma/extensions/Loop.ttdylib /usr/local/lib/Loop.ttdylib`
    `ln -s /usr/local/jamoma/extensions/Scenario.ttdylib /usr/local/lib/Scenario.ttdylib`
    
    # Clean .orig files
    `rm /usr/local/jamoma/extensions/*.orig`
    `rm /usr/local/jamoma/includes/*.orig`
    `rm /usr/local/jamoma/lib/*.orig`
end

puts "done"
puts

