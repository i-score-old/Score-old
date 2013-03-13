#!/usr/bin/env ruby -wKU
# encoding: utf-8

glibdir = "."
Dir.chdir glibdir
glibdir = Dir.pwd

projectNameParts = glibdir.split('/')
projectName = projectNameParts.last;
projectName.sub!(/Jamoma/, "")
ENV['JAMOMAPROJECT'] = projectName

Dir.chdir "#{glibdir}/../Shared"
load "build.rb"

puts "post-build..."
Dir.chdir "#{glibdir}"

# Copy Foundation and Modular headers and dylib into the support/jamoma folder to allows to build without all the Jamoma repository
if  win?
    
elsif mac?
    
    unless File.exist?("/usr/local/include")
        puts
        puts "Need Password to create an directories directory into /usr/local/include"
        puts "==================================================="
        puts
        `sudo mkdir -p /usr/local/include`
        `sudo chgrp admin /usr/local/include`
        `sudo chmod g+w /usr/local/include`
    end
    unless File.exist?("/usr/local/include/DeviceManager")
    	`sudo mkdir -p /usr/local/include/DeviceManager`
    	`sudo chgrp admin /usr/local/include/DeviceManager`
    	`sudo chmod g+w /usr/local/include/DeviceManager`
    	puts
    end
    unless File.exist?("/usr/local/include/IScore")
    	`sudo mkdir -p /usr/local/include/IScore`
    	`sudo chgrp admin /usr/local/include/IScore`
    	`sudo chmod g+w /usr/local/include/IScore`
    	puts
    end

    # Foundation and Modular extensions
    `cp /usr/local/jamoma/extensions/AnalysisLib.ttdylib "#{glibdir}"/support/jamoma/extensions`
    `cp /usr/local/jamoma/extensions/DataspaceLib.ttdylib "#{glibdir}"/support/jamoma/extensions`
    `cp /usr/local/jamoma/extensions/FunctionLib.ttdylib "#{glibdir}"/support/jamoma/extensions`
    `cp /usr/local/jamoma/extensions/NetworkLib.ttdylib "#{glibdir}"/support/jamoma/extensions`
    `cp /usr/local/jamoma/extensions/Minuit.ttdylib "#{glibdir}"/support/jamoma/extensions`
    `cp /usr/local/jamoma/extensions/OSC.ttdylib "#{glibdir}"/support/jamoma/extensions`
    
    # Foundation includes
    `cp "#{glibdir}"/../Foundation/library/includes/* "#{glibdir}"/support/jamoma/includes`
    
    # DSP includes
    `cp "#{glibdir}"/../DSP/library/includes/* "#{glibdir}"/support/jamoma/includes`
    
    # Modular includes
    `cp "#{glibdir}"/../../Modules/Modular/library/includes/TTModular.h "#{glibdir}"/support/jamoma/includes`
    `cp "#{glibdir}"/../../Modules/Modular/library/includes/TTModularSymbolCache.h "#{glibdir}"/support/jamoma/includes`
    
    `cp "#{glibdir}"/../../Modules/Modular/library/PeerObject/*.h "#{glibdir}"/support/jamoma/includes`
    
    `cp "#{glibdir}"/../../Modules/Modular/library/ProtocolLib/Protocol.h "#{glibdir}"/support/jamoma/includes`
    `cp "#{glibdir}"/../../Modules/Modular/library/SchedulerLib/Scheduler.h "#{glibdir}"/support/jamoma/includes`

    # Modular sources (used to build our own ECOMachine Scheduler outside the Modular folder)
    `cp "#{glibdir}"/../../Modules/Modular/library/SchedulerLib/Scheduler.cpp "#{glibdir}"/support/jamoma/source`
    
    # Foundation and Modular lib
    `cp /usr/local/jamoma/lib/JamomaFoundation.dylib "#{glibdir}"/support/jamoma/lib`
    `cp /usr/local/jamoma/lib/JamomaDSP.dylib "#{glibdir}"/support/jamoma/lib`
    `cp /usr/local/jamoma/lib/JamomaModular.dylib "#{glibdir}"/support/jamoma/lib`
    
    # Copy Score headers to include them into other application
    # (except the includes folder because it is done by the support/build.rb script)
    #`cp "#{glibdir}"/library/PeerObject/*.h /usr/local/jamoma/includes`
    
end

puts "done"
puts

