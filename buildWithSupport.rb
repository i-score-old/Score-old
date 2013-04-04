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
    
    # Copy support/jamoma folder into /usr/local/jamoma folder
    `cp "#{glibdir}"/support/jamoma/extensions/* /usr/local/jamoma/extensions`
    `cp "#{glibdir}"/support/jamoma/includes/* /usr/local/jamoma/includes`
    `cp "#{glibdir}"/support/jamoma/lib/* /usr/local/jamoma/lib`
    
    # Copy Score headers to include them into other application
    # (except the includes folder because it is done by the support/build.rb script) 
    `cp "#{glibdir}"/library/TimeProcessLib/*.h /usr/local/jamoma/includes`
    `cp "#{glibdir}"/library/TimeEventLib/*.h /usr/local/jamoma/includes`
    `cp "#{glibdir}"/library/tests/*.h /usr/local/jamoma/includes`
    
end

puts "done"
puts

