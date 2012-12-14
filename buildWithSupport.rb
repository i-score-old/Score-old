#!/usr/bin/env ruby -wKU
# encoding: utf-8

glibdir = "."
Dir.chdir glibdir
glibdir = Dir.pwd

projectNameParts = glibdir.split('/')
projectName = projectNameParts.last;
ENV['JAMOMAPROJECT'] = projectName

Dir.chdir "#{glibdir}/support"
load "build.rb"

puts "post-build..."
Dir.chdir "#{glibdir}"

if  win32?
    
else
    
    # Copy support/jamoma folder into /usr/local/jamoma folder
    `cp "#{glibdir}"/support/jamoma/extensions/* /usr/local/jamoma/extensions`
    `cp "#{glibdir}"/support/jamoma/includes/* /usr/local/jamoma/includes`
    `cp "#{glibdir}"/support/jamoma/lib/* /usr/local/jamoma/lib`
    
    # While Score is still based on DeviceManager we need to copy some files from the support/virage folder into some /usr/local folders
    `sudo cp "#{glibdir}"/support/virage/includes/* /usr/local/include/DeviceManager`
    `sudo cp "#{glibdir}"/support/virage/lib/libDeviceManager.a /usr/local/lib`
    `sudo cp "#{glibdir}"/support/virage/lib/Minuit.dylib /usr/local/lib/IScore`
    
    # Copy Score headers to include them into other application
    # (except the includes fiolder because it is done by the support/build.rb script) 
    `cp "#{glibdir}"/library/PeerObject/*.h /usr/local/jamoma/includes`
    
end

puts "done"
puts

