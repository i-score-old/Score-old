#!/usr/bin/env ruby -wKU
# encoding: utf-8

glibdir = "."
Dir.chdir glibdir
glibdir = Dir.pwd

projectNameParts = glibdir.split('/')
projectName = projectNameParts.last;
ENV['JAMOMAPROJECT'] = projectName

puts "pre-build..."
Dir.chdir "#{glibdir}/support"
load "jamomalib.rb"

if  win?
    
elsif mac?
    
    # Clean /usr/local/jamoma folder
    `rm /usr/local/jamoma/extensions/*.*`
    `rm /usr/local/jamoma/includes/*.*`
    `rm /usr/local/jamoma/lib/*.*`
    
end

Dir.chdir "#{glibdir}/support"
load "build.rb"

puts "post-build..."
Dir.chdir "#{glibdir}"

if  win?
    
elsif mac?
    
    # Copy support/jamoma folder into /usr/local/jamoma folder
    `cp -f -p ./support/jamoma/extensions/* /usr/local/jamoma/extensions`
    `cp -f -p ./support/jamoma/includes/* /usr/local/jamoma/includes`
    `cp -f -p ./support/jamoma/lib/* /usr/local/jamoma/lib`
    
    # Create alias
    `ln -s /usr/local/jamoma/lib/JamomaFoundation.dylib /usr/local/lib/JamomaFoundation.dylib`
    `ln -s /usr/local/jamoma/lib/JamomaDSP.dylib /usr/local/lib/JamomaDSP.dylib`
    `ln -s /usr/local/jamoma/lib/JamomaModular.dylib /usr/local/lib/JamomaModular.dylib`
    
    `ln -s /usr/local/jamoma/extensions/AnalysisLib.ttdylib /usr/local/lib/AnalysisLib.ttdylib`
    `ln -s /usr/local/jamoma/extensions/FunctionLib.ttdylib /usr/local/lib/FunctionLib.ttdylib`
    `ln -s /usr/local/jamoma/extensions/NetworkLib.ttdylib /usr/local/lib/NetworkLib.ttdylib`
    `ln -s /usr/local/jamoma/extensions/System.ttdylib /usr/local/lib/System.ttdylib`
    `ln -s /usr/local/jamoma/extensions/DataspaceLib.ttdylib /usr/local/lib/DataspaceLib.ttdylib`
    `ln -s /usr/local/jamoma/extensions/Minuit.ttdylib /usr/local/lib/Minuit.ttdylib`
    `ln -s /usr/local/jamoma/extensions/OSC.ttdylib /usr/local/lib/OSC.ttdylib`
    
    # Copy Score headers to include them into other application
    # Don't need to copy Score dylibs because the copy step is in the Makefile
    `cp -f -p ./library/includes/*.h /usr/local/jamoma/includes`
    `cp -f -p ./library/tests/*.h /usr/local/jamoma/includes`
    `cp -f -p ./extensions/TimePluginLib.h /usr/local/jamoma/includes`
    
    # Create alias
    `ln -s /usr/local/jamoma/lib/JamomaScore.dylib /usr/local/lib/JamomaScore.dylib`
    `ln -s /usr/local/jamoma/extensions/Automation.ttdylib /usr/local/lib/Automation.ttdylib`
    `ln -s /usr/local/jamoma/extensions/Interval.ttdylib /usr/local/lib/Interval.ttdylib`
    `ln -s /usr/local/jamoma/extensions/Scenario.ttdylib /usr/local/lib/Scenario.ttdylib`
    
end

puts "done"
puts

