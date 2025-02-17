eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
     & eval 'exec perl -S $0 $argv:q'
     if 0;

# $Id: run_test.pl 83699 2008-11-12 19:06:40Z johnnyw $
# -*- perl -*-

use lib "$ENV{ACE_ROOT}/bin";
use PerlACE::TestTarget;

$status = 0;

$synchbase = "ready";
my $target1 = PerlACE::TestTarget::create_target (1) || die "Create target 1 failed\n";
my $target2 = PerlACE::TestTarget::create_target (2) || die "Create target 2 failed\n";
$synchfile = $target1->LocalFile ("$synchbase");
my $port = $target1->RandomPort ();
my $host = $target1->HostName();

my $SV = $target1->CreateProcess("server", "-p $port -o $synchfile");
my $CL = $target2->CreateProcess ("client", " -h $host -p $port");

$target1->DeleteFile ($synchbase);
$target2->DeleteFile ($synchbase);

$SV->Spawn ();

if ($target1->WaitForFileTimed ($synchbase,
                                $target1->ProcessStartWaitInterval()) == -1) {
    print STDERR "ERROR: cannot find file <$synchfile>\n";
    $SV->Kill (); $SV->TimedWait (1);
    exit 1;
}
$target1->DeleteFile ($synchbase);

$client = $CL->SpawnWaitKill (300);

if ($client != 0) {
    print STDERR "ERROR: client returned $client\n";
    $status = 1;
}

$server = $SV->WaitKill (10);

if ($server != 0) {
    print STDERR "ERROR: server returned $server\n";
    $status = 1;
}

$target1->GetStderrLog();
$target2->GetStderrLog();

$target1->DeleteFile ($synchbase);
$target2->DeleteFile ($synchbase);

exit $status;
