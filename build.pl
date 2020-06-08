#!/usr/bin/perl

die "Must run as root to execute 'make install' for libwebsocket library\n" if $> != 0;

use File::Path qw(rmtree);
use File::Find;
use File::Basename;
use File::Copy;
use File::Copy::Recursive qw(fcopy rcopy dircopy fmove rmove dirmove);

use IPC::Cmd qw(can_run run);
use Cwd qw(cwd);

use Time::HiRes qw( time );

my $cmake_path;
my $make_path;
my $clang_path;
my $perl_script_path = cwd;
my $npm_path;

my $do_clean = 1;
my $no_lint = 0;

foreach (@ARGV) {
  if ("$_" eq "-update") {
    $do_clean = 0;
  } elsif ("$_" eq "-no-lint") {
    $no_lint = 1;
  }
}

sub VerifyPrerequisits {
  $cmake_path = can_run('cmake') or die 'cmake is not installed!';
  $make_path = can_run('make') or die 'cmake is not installed!';
  $clang_path = can_run('clang') or die 'clang is not installed!';
  $npm_path = can_run('npm') or  die 'npm is not installed!';
}

# Removes all used folders and builds the folder tree again
sub Clean {
  rmtree("out/");
  rmtree("build/");


  mkdir("out/");
  mkdir("out/web");

  mkdir("build/");
}

sub BuildRiskiServer {
  chdir($perl_script_path);
  chdir('build/');

  # Set compiler to clang
  $ENV{'CC'} = $clang_path;

  run(command => "$cmake_path ..",
      verbose => 1) or die;
  run(command => "$make_path",
      verbose => 1) or die;
}

sub LintTypeScript {
  if (-f "$_") {
    my ($name, $dir, $ext) = fileparse("$_", qw(.ts));
    if ($ext == ".ts") {
      run(command => "./node_modules/.bin/eslint $_", verbose => 1) or die;
    } else {
      warn "$_ found in ts folder probably should not be there";
    }
  }
}

sub BuildFrontEnd {
  chdir($perl_script_path);
  chdir("web/");

  run(command => "$npm_path update",
      verbose => 1) or die;
  run(command => "$npm_path install",
      verbose => 1) or die;

  if ($no_lint) {
    warn "WARNING: no typescript linting performed, this code may not be acceptable";
  } else {
    find({
          wanted => \&LintTypeScript,
          no_chdir => 1
         }, "ts/");
   }
  
  run(command => "./node_modules/.bin/tsc", verbose=>1) or die;
  run(command => "./node_modules/.bin/uglifyjs main.js -o main.min.js",
      verbose=>1) or die;
}

sub TransferToOut {
  chdir($perl_script_path);
  copy("./build/src/riski", "out/") or die;
  chmod(0755, "./out/riski");
  copy("./symbols.csv", "out/") or die;
  copy("./web/main.min.js", "out/web/") or die;
  copy("./web/index.html", "out/web/") or die;
  dircopy("./web/img/", "out/web/img/") or die;
}

my $begin_time = time();

VerifyPrerequisits;

if ($do_clean == 1) {
  Clean;
}
BuildRiskiServer;
BuildFrontEnd;
TransferToOut;

my $end_time = time();
printf("BUILD TIME ELAPSED %.2fs\n", $end_time - $begin_time);