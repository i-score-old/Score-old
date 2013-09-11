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

if  win32?
    
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

if  win32?
    
elsif mac?
    
    # Copy support/jamoma folder into /usr/local/jamoma folder
    `sudo cp "#{glibdir}"/support/jamoma/extensions/* /usr/local/jamoma/extensions`
    `sudo cp "#{glibdir}"/support/jamoma/includes/* /usr/local/jamoma/includes`
    `sudo cp "#{glibdir}"/support/jamoma/lib/* /usr/local/jamoma/lib`
    
    # Create alias
    `sudo ln -s /usr/local/jamoma/lib/JamomaFoundation.dylib /usr/local/lib/JamomaFoundation.dylib`
    `sudo ln -s /usr/local/jamoma/lib/JamomaDSP.dylib /usr/local/lib/JamomaDSP.dylib`
    `sudo ln -s /usr/local/jamoma/lib/JamomaModular.dylib /usr/local/lib/JamomaModular.dylib`
    
    `sudo ln -s /usr/local/jamoma/extensions/AnalysisLib.ttdylib /usr/local/lib/AnalysisLib.ttdylib`
    `sudo ln -s /usr/local/jamoma/extensions/FunctionLib.ttdylib /usr/local/lib/FunctionLib.ttdylib`
    `sudo ln -s /usr/local/jamoma/extensions/NetworkLib.ttdylib /usr/local/lib/NetworkLib.ttdylib`
    `sudo ln -s /usr/local/jamoma/extensions/System.ttdylib /usr/local/lib/System.ttdylib`
    `sudo ln -s /usr/local/jamoma/extensions/DataspaceLib.ttdylib /usr/local/lib/DataspaceLib.ttdylib`
    `sudo ln -s /usr/local/jamoma/extensions/Minuit.ttdylib /usr/local/lib/Minuit.ttdylib`
    `sudo ln -s /usr/local/jamoma/extensions/OSC.ttdylib /usr/local/lib/OSC.ttdylib`
    
    # Copy Score headers to include them into other application
    # (except the includes folder because it is done by the support/build.rb script)
    `sudo cp "#{glibdir}"/library/tests/*.h /usr/local/jamoma/includes`
    `sudo cp "#{glibdir}"/extensions/TimePluginLib.h /usr/local/jamoma/includes`
    
    # Create alias
    `sudo ln -s /usr/local/jamoma/extensions/Automation.ttdylib /usr/local/lib/Automation.ttdylib`
    `sudo ln -s /usr/local/jamoma/extensions/Condition.ttdylib /usr/local/lib/Condition.ttdylib`
    `sudo ln -s /usr/local/jamoma/extensions/Interval.ttdylib /usr/local/lib/Interval.ttdylib`
    `sudo ln -s /usr/local/jamoma/extensions/Scenario.ttdylib /usr/local/lib/Scenario.ttdylib`
    
end

puts "done"
puts

