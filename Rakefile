require 'find'
require 'erb'
require 'rake/rdoctask'

#Look, another big fat hack. Make it so we can remove tasks from rake -T by setting comment to nil
module Rake
  class Task
    attr_accessor :comment
  end
end


chdir File.dirname(__FILE__)

require 'lib/build/jake.rb'

load 'platform/bb/build/bb.rake'
load 'platform/android/build/android.rake'
load 'platform/iphone/rbuild/iphone.rake'
load 'platform/wm/build/wm.rake'
load 'platform/linux/tasks/linux.rake'


namespace "framework" do
  task :spec do
    loadpath = $LOAD_PATH.inject("") { |load_path,pe| load_path += " -I" + pe }

    rhoruby = ""

    if RUBY_PLATFORM =~ /(win|w)32$/
      rhoruby = 'res\\build-tools\\RhoRuby'
    elsif RUBY_PLATFORM =~ /darwin/
      rhoruby = 'res/build-tools/RubyMac'
    else
      rhoruby = 'res/build-tools/rubylinux'
    end
   
    puts `#{rhoruby}  -I#{File.expand_path('spec/framework_spec/app/')} -I#{File.expand_path('lib/framework')} -I#{File.expand_path('lib/test')} -Clib/test framework_test.rb`
  end
end


namespace "config" do
  task :common do
    $startdir = File.dirname(__FILE__)
    $binextensions = []
    buildyml = 'rhobuild.yml'

    buildyml = ENV["RHOBUILD"] unless ENV["RHOBUILD"].nil?
    $config = Jake.config(File.open(buildyml))
    if RUBY_PLATFORM =~ /(win|w)32$/
      $all_files_mask = "*.*"
    else
      $all_files_mask = "*"
    end
    if $app_path.nil? #if we are called from the rakefile directly, this wont be set
      #load the apps path and config

      $app_path = $config["env"]["app"]
      unless File.exists? $app_path
        puts "Could not find rhodes application. Please verify your application setting in #{File.dirname(__FILE__)}/rhobuild.yml"
        exit 1
      end
      $app_config = YAML::load_file($app_path + "/build.yml")

    end
    Jake.set_bbver($app_config["bbver"].to_s)
  end

  out = `javac -version 2>&1`
  puts "\n\nYour java bin folder does not appear to be on your path.\nThis is required to use rhodes.\n\n" unless $? == 0
end

def copy_assets(asset)
  
  dest = File.join($srcdir,'apps/public')
  
  cp_r asset + "/.", dest, :remove_destination => true 
  
end

def check_extension_file
  extfile = ""
  File.open($startdir + "/platform/shared/ruby/ext/rho/extensions.c","r") do |f|
    f.each_line do |line|
      if line !~ /;/
        extfile << line
      else
        loaded = false
        $binextensions.each  { |loadedext| loaded = line.include? loadedext; break if loaded }
        extfile << line if loaded

      end

    end
  end
  if extfile != ""
    File.open($startdir + "/platform/shared/ruby/ext/rho/extensions.c","w") { |f| f.write extfile }
  end
end

def clear_linker_settings
  if $config["platform"] == "iphone"
#    outfile = ""
#    IO.read($startdir + "/platform/iphone/rhorunner.xcodeproj/project.pbxproj").each_line do |line|
#      if line =~ /EXTENSIONS_LDFLAGS = /
#        outfile << line.gsub(/EXTENSIONS_LDFLAGS = ".*"/, 'EXTENSIONS_LDFLAGS = ""')
#      else
#        outfile << line
#      end
#    end
#    File.open($startdir + "/platform/iphone/rhorunner.xcodeproj/project.pbxproj","w") {|f| f.write outfile}
#    ENV["EXTENSIONS_LDFLAGS"] = ""

    $ldflags = ""
  end

end

def add_linker_library(libraryname)
#  if $config["platform"] == "iphone"
#    outfile = ""
#    IO.read($startdir + "/platform/iphone/rhorunner.xcodeproj/project.pbxproj").each_line do |line|
#      if line =~ /EXTENSIONS_LDFLAGS = /
#        outfile << line.gsub(/";/, " $(TARGET_TEMP_DIR)/#{libraryname}\";")
#      else
#        outfile << line
#      end
#    end
#    File.open($startdir + "/platform/iphone/rhorunner.xcodeproj/project.pbxproj","w") {|f| f.write outfile}
#  end
      simulator = $sdk =~ /iphonesimulator/

      if ENV["TARGET_TEMP_DIR"] and ENV["TARGET_TEMP_DIR"] != ""
        tmpdir = ENV["TARGET_TEMP_DIR"]
      else
        tmpdir = $startdir + "/platform/iphone/build/rhorunner.build/#{$configuration}-" +
          ( simulator ? "iphonesimulator" : "iphoneos") + "/rhorunner.build"
      end
  $ldflags << "#{tmpdir}/#{libraryname}\n" unless $ldflags.nil?
end

def set_linker_flags
  if $config["platform"] == "iphone"
      simulator = $sdk =~ /iphonesimulator/
      if ENV["TARGET_TEMP_DIR"] and ENV["TARGET_TEMP_DIR"] != ""
        tmpdir = ENV["TARGET_TEMP_DIR"]
      else
        tmpdir = $startdir + "/platform/iphone/build/rhorunner.build/#{$configuration}-" +
          ( simulator ? "iphonesimulator" : "iphoneos") + "/rhorunner.build"
      end
      mkdir_p tmpdir unless File.exist? tmpdir
      File.open(tmpdir + "/rhodeslibs.txt","w") { |f| f.write $ldflags }
#    ENV["EXTENSIONS_LDFLAGS"] = $ldflags
#    puts `export $EXTENSIONS_LDFLAGS`
  end

end

def add_extension(path,dest)
  start = pwd
  chdir path if File.directory?(path)

  Dir.glob("*").each { |f| cp_r f,dest unless f =~ /^ext(\/|(\.yml)?$)/ }

  if File.exist? "ext.yml"
    extension_config = YAML::load_file("ext.yml")

    if extension_config["entry"] and extension_config["entry"] != ""
      extfile = ""
      File.open($startdir + "/platform/shared/ruby/ext/rho/extensions.c","r") do |f|
        externstart = false
        externwritten = false
        callstart = false
        callwritten = false

        f.each_line do |line|
   #       puts line
          #are we starting a replacement area?
          externstart = true if line =~ /EXTERNS/
          callstart = true if line =~ /CALLS/


          #if we arent in our replacement area, just copy the line
          unless externstart or callstart
            extfile << line
          else
            #always write an end marker
            extfile << line if line =~ /END/
            #did we just start the extern replacement area?
            if externstart and not externwritten
              #write marker and our new extension
              extfile << line
              extfile << "extern void #{extension_config["entry"]}(void);\n"
              externwritten = true
            end

            #same for calls
            if callstart and not callwritten
              extfile << line
              extfile << "#{extension_config["entry"]}();\n"
              callwritten = true
            end

            #did we leave a replacement area
            externstart = false if externstart and line =~ /END/
            callstart = false if callstart and line =~ /END/

            #if we are in a replacement area, check for lines that are there
            #that we have marked as loaded and copy those over
            #this is to make sure we are only loading things we explicitly marked
            #leaving out lines that came from a previous run
            if externstart or callstart
              loaded = false
              $binextensions.each  { |loadedext| loaded = line.include? loadedext; break if loaded }
              extfile << line if loaded
            end
          end

        end
        $binextensions << extension_config["entry"]
      end

      if extfile != ""
        File.open($startdir + "/platform/shared/ruby/ext/rho/extensions.c","w") { |f| f.write extfile }
      end
      
    end

    if extension_config["libraries"] and extension_config["libraries"].is_a? Array
      extension_config["libraries"].each { |lib| add_linker_library(lib) }
    end
  end

  chdir start

end

def common_bundle_start(startdir, dest)
  app = $app_path
  rhodeslib = "lib/framework"

  rm_rf $srcdir
  mkdir_p $srcdir
  mkdir_p dest if not File.exists? dest
  mkdir_p File.join($srcdir,'apps')


  start = pwd
  chdir rhodeslib

  Dir.glob("*").each { |f| cp_r f,dest }

  chdir dest
  Dir.glob("**/rhodes-framework.rb").each {|f| rm f}
  Dir.glob("**/erb.rb").each {|f| rm f}
  Dir.glob("**/find.rb").each {|f| rm f}
  $excludelib.each {|e| Dir.glob(e).each {|f| rm f}}

  chdir start
  clear_linker_settings

  extensions = []
  extensions += $app_config["extensions"] if $app_config["extensions"] and
    $app_config["extensions"].is_a? Array
  extensions += $app_config[$config["platform"]["extensions"]] if $config["platform"] and
    $config["platform"]["extensions"] and $config["platform"]["extensions"].is_a? Array
  $app_config["extensions"] = extensions
    
  $app_config["extensions"].each do |extname|
    rhoextpath = "lib/extensions/" + extname
    appextpath = $app_path + "/extensions/" + extname
    extpath = nil

    if File.exists? appextpath
      extpath = appextpath
    elsif File.exists? rhoextpath
      extpath = rhoextpath
    end

    unless extpath.nil?
      add_extension(extpath, dest)
    end

  end

  check_extension_file
  set_linker_flags

  unless $app_config["constants"].nil?
    File.open("rhobuild.rb","w") do |file|
      file << "module RhoBuild\n"
      $app_config["constants"].each do |key,value|
        value.gsub!(/"/,"\\\"")
        file << "  #{key.upcase} = \"#{value}\"\n"
      end
      file << "end\n"
    end
  end

  chdir startdir
  #throw "ME"
  cp_r app + '/app',File.join($srcdir,'apps')
  cp_r app + '/public', File.join($srcdir,'apps')
  cp   app + '/rhoconfig.txt', File.join($srcdir,'apps')

  copy_assets($assetfolder) if ($assetfolder and File.exists? $assetfolder)

  chdir File.join($srcdir,'apps')

  Dir.glob("**/*.#{$config['platform']}.*").each do |file|
    oldfile = file.gsub(Regexp.new(Regexp.escape('.') + $config['platform'] + Regexp.escape('.')),'.')
    rm oldfile if File.exists? oldfile
    mv file,oldfile
  end
  
  Dir.glob("**/*.wm.*").each { |f| rm f }
  Dir.glob("**/*.iphone.*").each { |f| rm f }
  Dir.glob("**/*.bb.*").each { |f| rm f }
  Dir.glob("**/*.android.*").each { |f| rm f }

end

def create_manifest
  dir = File.join($srcdir, 'apps')
  fname = "config.rb"
  fappManifest = File.new( File.join(dir,'app_manifest.txt'), "w")

  Find.find(dir) do |path|
    if File.basename(path) == fname

      relPath = path[dir.length+1, File.dirname(path).length-1]   #relative path
      relPath = relPath[0, relPath.length-3] #remove .rb extension
      fappManifest.puts( relPath )

    end
  end

  fappManifest.close()
  
end
  
namespace "build" do
  namespace "bundle" do
    task :xruby do
      #needs $config, $srcdir, $excludelib, $bindir
      app = $app_path
      startdir = pwd
      dest =  $srcdir
      xruby =  File.dirname(__FILE__) + '/res/build-tools/xruby-0.3.3.jar'
      compileERB = "lib/build/compileERB/bb.rb"
      rhodeslib = File.dirname(__FILE__) + "/lib/framework"
      
      common_bundle_start(startdir,dest)

      if not $config["excludedirs"].nil?
        if $config["excludedirs"].has_key?($config["platform"])
          chdir File.join($srcdir, 'apps')

          excl = $config["excludedirs"][$config["platform"]]
          excl.each do |mask|
            Dir.glob(mask).each {|f| rm_rf f}
          end
        end
      end
      
      cp_r File.join(startdir, "res/build-tools/db"), File.join($srcdir, 'apps')
      
      chdir startdir
      
      #create manifest
      create_manifest
      
      #"compile ERB"
      #ext = ".erb"
      #Find.find($srcdir) do |path|
      #  if File.extname(path) == ext
      #    rbText = ERB.new( IO.read(path) ).src
      #    newName = File.basename(path).sub('.erb','_erb.rb')
      #    fName = File.join(File.dirname(path), newName)
      #    frb = File.new(fName, "w")
      #    frb.write( rbText )
      #    frb.close()
      #  end
      #end
      cp   compileERB, $srcdir
      puts "Running bb.rb"

      puts `#{$rubypath} -I#{rhodeslib} "#{$srcdir}/bb.rb"`
      unless $? == 0
        puts "Error interpreting erb code"
        exit 1
      end

      rm "#{$srcdir}/bb.rb"

      chdir $bindir
      # -n#{$bundleClassName}
      output = `java -jar "#{xruby}" -v -c RhoBundle 2>&1`
      output.each_line { |x| puts ">>> " + x  }
      unless $? == 0
        puts "Error interpreting ruby code"
        exit 1
      end
      chdir startdir
      chdir $srcdir
  
      Dir.glob("**/*.rb") { |f| rm f }
      Dir.glob("**/*.erb") { |f| rm f }

      # RubyIDContainer.* files takes half space of jar why we need it?
      #Jake.unjar("../RhoBundle.jar", $tmpdir)
      #Dir.glob($tmpdir + "/**/RubyIDContainer.class") { |f| rm f }
      #rm "#{$bindir}/RhoBundle.jar"
      #chdir $tmpdir
      #puts `jar cf #{$bindir}/RhoBundle.jar #{$all_files_mask}`      
      #rm_rf $tmpdir
      #mkdir_p $tmpdir
      #chdir $srcdir
      
      puts `jar uf ../RhoBundle.jar apps/#{$all_files_mask}`
      unless $? == 0
        puts "Error creating Rhobundle.jar"
        exit 1
      end
      chdir startdir
      
    end

    task :noxruby do
      app = $app_path
      rhodeslib = File.dirname(__FILE__) + "/lib/framework"
      compileERB = "lib/build/compileERB/default.rb"
      compileRB = "lib/build/compileRB/compileRB.rb"
      startdir = pwd
      dest = $srcdir + "/lib"      

      common_bundle_start(startdir,dest)
      chdir startdir
      
      create_manifest
      
      cp   compileERB, $srcdir
      puts "Running default.rb"

      puts `#{$rubypath} -I#{rhodeslib} "#{$srcdir}/default.rb"`
      unless $? == 0
        puts "Error interpreting erb code"
        exit 1
      end

      rm "#{$srcdir}/default.rb"

      cp   compileRB, $srcdir
      puts "Running compileRB"
      puts `#{$rubypath} -I#{rhodeslib} "#{$srcdir}/compileRB.rb"`
      unless $? == 0
        puts "Error interpreting ruby code"
        exit 1
      end

      chdir $srcdir
      Dir.glob("**/*.rb") { |f| rm f }
      Dir.glob("**/*.erb") { |f| rm f }
  
      chdir startdir

      cp_r "res/build-tools/db", $srcdir 
    end
  end
end


# Simple rakefile that loads subdirectory 'rhodes' Rakefile
# run "rake -T" to see list of available tasks

#desc "Get versions"
task :get_version do

  genver = "unknown"
  iphonever = "unknown"
  #symver = "unknown"
  wmver = "unknown"
  androidver = "unknown"
  

  File.open("res/generators/templates/application/build.yml","r") do |f|
    file = f.read
    if file.match(/version: (\d+\.\d+\.\d+)/)
      genver = $1
    end
  end

  File.open("platform/iphone/Info.plist","r") do |f|
    file = f.read
    if file.match(/CFBundleVersion<\/key>\s+<string>(\d+\.\d+\.*\d*)<\/string>/)
      iphonever =  $1
    end
  end

  # File.open("platform/symbian/build/release.properties","r") do |f|
  #     file = f.read
  #     major = ""
  #     minor = ""
  #     build = ""
  # 
  #     if file.match(/release\.major=(\d+)/)
  #       major =  $1
  #     end
  #     if file.match(/release\.minor=(\d+)/)
  #       minor =  $1
  #     end
  #     if file.match(/build\.number=(\d+)/)
  #       build =  $1
  #     end
  # 
  #     symver = major + "." + minor + "." + build
  #   end

  File.open("platform/android/Rhodes/AndroidManifest.xml","r") do |f|
    file = f.read
    if file.match(/versionName="(\d+\.\d+\.*\d*)"/)
      androidver =  $1
    end
  end

  gemver = "unknown"
  rhodesver = "unknown"
  frameworkver = "unknown"

  File.open("lib/rhodes.rb","r") do |f|
    file = f.read
    if file.match(/VERSION = '(\d+\.\d+\.*\d*)'/)
      gemver =  $1
    end
  end

  File.open("lib/framework/rhodes.rb","r") do |f|
    file = f.read
    if file.match(/VERSION = '(\d+\.\d+\.*\d*)'/)
      rhodesver =  $1
    end
  end

  File.open("lib/framework/version.rb","r") do |f|
    file = f.read
    if file.match(/VERSION = '(\d+\.\d+\.*\d*)'/)
      frameworkver =  $1
    end
  end

  

  puts "Versions:"
  puts "  Generator:        " + genver
  puts "  iPhone:           " + iphonever
  #puts "  Symbian:          " + symver
  #puts "  WinMo:            " + wmver
  puts "  Android:          " + androidver
  puts "  Gem:              " + gemver
  puts "  Rhodes:           " + rhodesver
  puts "  Framework:        " + frameworkver
end

#desc "Set version"
task :set_version, [:version] do |t,args|
  throw "You must pass in version" if args.version.nil?
  ver = args.version.split(/\./)
  major = ver[0]
  minor = ver[1]
  build = ver[2]
  
  throw "Invalid version format. Must be in the format of: major.minor.build" if major.nil? or minor.nil? or build.nil?

  verstring = major+"."+minor+"."+build
  origfile = ""

  File.open("res/generators/templates/application/build.yml","r") { |f| origfile = f.read }
  File.open("res/generators/templates/application/build.yml","w") do |f|
    f.write origfile.gsub(/version: (\d+\.\d+\.\d+)/, "version: #{verstring}")
  end
  

  File.open("platform/iphone/Info.plist","r") { |f| origfile = f.read }
  File.open("platform/iphone/Info.plist","w") do |f| 
    f.write origfile.gsub(/CFBundleVersion<\/key>(\s+)<string>(\d+\.\d+\.*\d*)<\/string>/, "CFBundleVersion</key>\n  <string>#{verstring}</string>")
  end

  # File.open("platform/symbian/build/release.properties","r") { |f| origfile = f.read }
  # File.open("platform/symbian/build/release.properties","w") do |f|
  #   origfile.gsub!(/release\.major=(\d+)/,"release.major=#{major}")
  #   origfile.gsub!(/release\.minor=(\d+)/,"release.minor=#{minor}")
  #   origfile.gsub!(/build\.number=(\d+)/,"build.number=#{build}")
  #   f.write origfile
  # end

  File.open("platform/android/Rhodes/AndroidManifest.xml","r") { |f| origfile = f.read }
  File.open("platform/android/Rhodes/AndroidManifest.xml","w") do |f|
    origfile.match(/versionCode="(\d+)"/)
    vercode = $1.to_i + 1
    origfile.gsub!(/versionCode="(\d+)"/,"versionCode=\"#{vercode}\"")
    origfile.gsub!(/versionName="(\d+\.\d+\.*\d*)"/,"versionName=\"#{verstring}\"")

    f.write origfile
  end

  ["lib/rhodes.rb","lib/framework/rhodes.rb","lib/framework/version.rb"].each do |versionfile|
  
    File.open(versionfile,"r") { |f| origfile = f.read }
    File.open(versionfile,"w") do |f|
      origfile.gsub!(/VERSION = '(\d+\.\d+\.*\d*)'/, "VERSION = '#{verstring}'")
      origfile.gsub!(/DBVERSION = '(\d+\.\d+\.*\d*)'/, "DBVERSION = '#{verstring}'")
      f.write origfile
    end
  end
  Rake::Task[:get_version].invoke  
end



namespace "buildall" do
  namespace "bb" do
    #    desc "Build all jdk versions for blackberry"
    task :production => "config:common" do
      $config["env"]["paths"].each do |k,v|
        if k.to_s =~ /^4/
          puts "BUILDING VERSION: #{k}"
          $app_config["bbver"] = k
          Jake.reconfig($config)
 
          #reset all tasks used for building
          Rake::Task["config:bb"].reenable
          Rake::Task["build:bb:rhobundle"].reenable
          Rake::Task["build:bb:rhodes"].reenable
          Rake::Task["build:bb:rubyvm"].reenable
          Rake::Task["device:bb:dev"].reenable
          Rake::Task["device:bb:production"].reenable
          Rake::Task["device:bb:rhobundle"].reenable
          Rake::Task["package:bb:dev"].reenable
          Rake::Task["package:bb:production"].reenable
          Rake::Task["package:bb:rhobundle"].reenable
          Rake::Task["package:bb:rhodes"].reenable
          Rake::Task["package:bb:rubyvm"].reenable
          Rake::Task["device:bb:production"].reenable
          Rake::Task["clean:bb:preverified"].reenable

          Rake::Task["clean:bb:preverified"].invoke
          Rake::Task["device:bb:production"].invoke
        end
      end

    end
  end
end

task :gem do
  puts "Removing old gem"
  rm_rf Dir.glob("*.gem")
  puts "Copying Rakefile"
  cp "Rakefile", "rakefile.rb"
  
  puts "Building manifest"
  out = ""
  Dir.glob("**/*") {|fname| out << fname + "\n" if File.file? fname}
  File.open("Manifest.txt",'w') {|f| f.write(out)}

  puts "Loading gemspec"
  spec = Gem::Specification.load('rhodes.gemspec')

  puts "Building gem"
  gemfile = Gem::Builder.new(spec).build
end

task :tasks do
  Rake::Task.tasks.each {|t| puts t.to_s.ljust(27) + "# " + t.comment.to_s}
end

task :switch_app => "config:common" do
  rhobuildyml = File.dirname(__FILE__) + "/rhobuild.yml"
  if File.exists? rhobuildyml
    config = YAML::load_file(rhobuildyml)
  else
    puts "Cant find rhobuild.yml"
    exit 1
  end
  config["env"]["app"] = $app_path.gsub(/\\/,"/")
  File.open(  rhobuildyml, 'w' ) do |out|
    YAML.dump( config, out )
  end
end


Rake::RDocTask.new do |rd|
  rd.main = "README.textile"
  rd.rdoc_files.include("README.textile", "lib/framework/**/*.rb")
end
Rake::Task["rdoc"].comment=nil
Rake::Task["rerdoc"].comment=nil

task :rdocpush => :rdoc do
  puts "Pushing RDOC. This may take a while"
  `scp -r html/* dev@dev.rhomobile.com:dev.rhomobile.com/rhodes/`
end
