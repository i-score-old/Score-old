#!/usr/bin/env ruby -wKU
# encoding: utf-8

glibdir = "."
Dir.chdir glibdir
glibdir = Dir.pwd

projectNameParts = glibdir.split('/')
projectName = projectNameParts.last;
projectName.gsub!(/Jamoma/, "")
ENV['JAMOMAPROJECT'] = projectName

Dir.chdir "#{glibdir}/../Shared"
load "build.rb"

puts "post-build..."
Dir.chdir "#{glibdir}"

# Copy Foundation and Modular headers and dylib into the support/jamoma folder to allows to build without all the Jamoma repository
if  win32?
    
    else
    
    # Foundation and Modular extensions
    `cp /usr/local/jamoma/extensions/AnalysisLib.ttdylib "#{glibdir}"/support/jamoma/extensions`
    `cp /usr/local/jamoma/extensions/DataspaceLib.ttdylib "#{glibdir}"/support/jamoma/extensions`
    `cp /usr/local/jamoma/extensions/Minuit.ttdylib "#{glibdir}"/support/jamoma/extensions`
    `cp /usr/local/jamoma/extensions/NetworkLib.ttdylib "#{glibdir}"/support/jamoma/extensions`
    
    # Foundation includes
    `cp "#{glibdir}"/../Foundation/library/includes/* "#{glibdir}"/support/jamoma/includes`
    
    # Modular includes
    `cp "#{glibdir}"/../../Modules/Modular/library/FunctionLib/FunctionLib.h "#{glibdir}"/support/jamoma/includes`
    
    `cp "#{glibdir}"/../../Modules/Modular/library/includes/TTModular.h "#{glibdir}"/support/jamoma/includes`
    `cp "#{glibdir}"/../../Modules/Modular/library/includes/TTModularSymbolCache.h "#{glibdir}"/support/jamoma/includes`
    
    `cp "#{glibdir}"/../../Modules/Modular/library/PeerObject/*.h "#{glibdir}"/support/jamoma/includes`
    
    `cp "#{glibdir}"/../../Modules/Modular/library/ProtocolLib/Protocol.h "#{glibdir}"/support/jamoma/includes`
    
    # Foundation and Modular lib
    `cp /usr/local/jamoma/lib/JamomaFoundation.dylib "#{glibdir}"/support/jamoma/lib`
    `cp /usr/local/jamoma/lib/JamomaModular.dylib "#{glibdir}"/support/jamoma/lib`
    
end

puts "done"
puts

