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

# Copy support/jamoma folder into /usr/local/jamoma folder
if  win32?
    
else
    
    `cp "#{glibdir}"/support/jamoma/extensions/* /usr/local/jamoma/extensions`
    `cp "#{glibdir}"/support/jamoma/includes/* /usr/local/jamoma/includes`
    `cp "#{glibdir}"/support/jamoma/lib/* /usr/local/jamoma/lib`
    
end

puts "done"
puts

