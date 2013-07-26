#! /usr/bin/perl
#
# QueenMaxima, a chess playing program. 
# Copyright (C) 1996-2013 Erik van het Hof and Hermen Reitsma 
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
# 
#
#
# Example startup script with emailing of core dumps.
#

sub check_core
  {

    # check if maxima generated a core file. 
    # if she did, email the stacktrace and rename the file. 

    if (-e 'core') {

      @time = localtime;
      $timestr = sprintf("%d-%02d-%02d-%02d-%02d",
			 $time[5]+1900, $time[4]+1, $time[3], $time[2], $time[1]);

      open(SENDMAIL,"|/usr/lib/sendmail -t");
      print SENDMAIL "From: maxima\@hofcom.org\n";
      print SENDMAIL "To: maxima\@hofcom.org\n";
      print SENDMAIL "Subject: [maxima] Core dumped and offline\n";
      print SENDMAIL "\n";
      print SENDMAIL "Maxima dumped core at: $timestr\n";
      print SENDMAIL "\ncore file and maxima with time prefix copied to dump directory\n"; 
      print SENDMAIL "\noutput of: gdb -batch -x gdbc src/maxima core\n"; 
      print SENDMAIL "\n";

      open(GDB,"/usr/bin/gdb -batch -x gdbc src/maxima core |");
      while ($line = <GDB>) {
	print SENDMAIL $line;
      }
      close(GDB);

      print SENDMAIL "\n\n---\n";
      close(SENDMAIL);

      # move/copy files 
      system("mv core dump/core$timestr"); 
      system("cp src/maxima dump/maxima$timestr");

      # and exit 
      exit(1);
    }
}

sub check_process
  {
    my $procname = $_[0];
    my @files = glob("/proc/*");

    foreach my $file (@files) {
      if ($file =~ m/^\/proc\/\d+$/) {
	local *FH;
	open(FH,"<$file/cmdline");
	if (<FH> =~ m/^$procname/) {
	  return 1;
	}
	close(FH);
      }
    }
    return 0;
}


# create dump directory 
system("mkdir dump") unless -d 'dump';

# kill all other gomaxima processes. assuming we have the rights. 
open(GMPL, "ps -A | grep gomaxima |"); 
while ($line = <GMPL>) {
	$line =~ /(\d+)/;
	if ($$ != $1) {  
		print "$0: killing other gomaxima.pl with pid $1\n";
		system("/bin/kill $1"); 
	} 
}
close(GMPL); 
# 
#while (check_process('maxima')==0) { 
#	print "$0: waiting for other maxima process to finish. \n";
#	sleep(300); 
#}

while (1==1) {

check_core();
if (check_process('timestamp')==0) {
  print "$0: Starting timestamp\n";
  open(LOGFILE,">> maxima.log"); 
  print LOGFILE scalar(localtime) . " timestamp started.\n";
  close(LOGFILE); 
  system("/usr/bin/schroot -c i386 -- /home/hof/timestamp_linux_2.6.8 207.99.83.228 5000 -p 5000 >/dev/null </dev/null &");
}

if (check_process('maxima')==0) {
  print "$0: starting maxima\n";
  open (LOGFILE, ">> maxima.log");
  print LOGFILE scalar(localtime) . " maxima started.\n";
  close(LOGFILE); 
  system("./src/maxima -icc-host=localhost -icc-user=QueenMaxima -icc-password=password -db-host=localhost -db-user=maxima -db-password=password -db-database=maxima icc");
}
print "$0: waiting for restart\n";
sleep(30);
}

