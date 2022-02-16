eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
     & eval 'exec perl -S $0 $argv:q'
     if 0;

# -*- perl -*-

use lib "$ENV{ACE_ROOT}/bin";
use PerlACE::TestTarget;

$status = 0;
$debug_level = '0';

# amount of delay between running the servers

$sleeptime = 1;
$no_naming_service = 0;

foreach $i (@ARGV) {
    if ($i eq '-s') {
        $no_naming_service = 1;
    }
    elsif ($i eq '-debug') {
        $debug_level = '3';
        $ENV{ACE_TEST_VERBOSE} = 1;
    }
}

my $nstarget = PerlACE::TestTarget::create_target (1) || die "Create tao_cosnaming failed\n";
my $ntarget = PerlACE::TestTarget::create_target (2) || die "Create notifer target failed\n";
my $c1target = PerlACE::TestTarget::create_target (3) || die "Create client target 1 failed\n";
my $c2target = PerlACE::TestTarget::create_target (3) || die "Create client target 2 failed\n";
my $starget = PerlACE::TestTarget::create_target (4) || die "Create supplier target failed\n";

my $nsiorbase = "ns.ior";
my $examplebase = "example.stocks";
my $nstarget_nsiorfile = $nstarget->LocalFile ($nsiorbase);
my $ntarget_nsiorfile = $ntarget->LocalFile ($nsiorbase);
my $c1target_nsiorfile = $c1target->LocalFile ($nsiorbase);
my $c2target_nsiorfile = $c1target->LocalFile ($nsiorbase);
my $starget_nsiorfile = $starget->LocalFile ($nsiorbase);
my $starget_examplefile = $starget->LocalFile ($examplebase);
$nstarget->DeleteFile($nsiorbase);
$ntarget->DeleteFile($nsiorbase);
$c1target->DeleteFile($nsiorbase);
$c2target->DeleteFile($nsiorbase);
$starget->DeleteFile($nsiorbase);

# Programs that are run

if ($no_naming_service)
{
    $N = $ntarget->CreateProcess (
        "notifier",
        "-ORBDebugLevel $debug_level ".
        "-f $ntarget_nsiorfile -s");
    $C1 = $c1target->CreateProcess (
        "consumer",
        "-ORBDebugLevel $debug_level ".
        "-f $ntarget_nsiorfile -s ".
        "-t 7 -a TAO");
    $S = $starget->CreateProcess (
        "supplier",
        "-ORBDebugLevel $debug_level ".
        "-f $ntarget_nsiorfile -s ".
        "-i $starget_examplefile");
    $C2 = $c2target->CreateProcess (
        "consumer",
        "-ORBDebugLevel $debug_level ".
        "-f $ntarget_nsiorfile -s ".
        "-t 11 -a ACE");
}
else
{
    $NS = $nstarget->CreateProcess (
        "$ENV{TAO_ROOT}/orbsvcs/Naming_Service/tao_cosnaming",
        "-o $nstarget_nsiorfile");
    $N = $ntarget->CreateProcess (
        "notifier",
        "-ORBDebugLevel $debug_level ".
        "-ORBInitRef NameService=file://$ntarget_nsiorfile");
    $C1 = $c1target->CreateProcess (
        "consumer",
        "-ORBDebugLevel $debug_level ".
        "-ORBInitRef NameService=file://$c1target_nsiorfile -t 6 -a TAO");
    $S = $starget->CreateProcess (
        "supplier",
        "-ORBDebugLevel $debug_level ".
        "-ORBInitRef NameService=file://$starget_nsiorfile -i $starget_examplefile");
    $C2 = $c2target->CreateProcess (
        "consumer",
        "-ORBDebugLevel $debug_level ".
        "-ORBInitRef NameService=file://$c1target_nsiorfile -t 12 -a TAO");
}

print STDERR "================ Remote test\n";

if ($no_naming_service == 0)
{
    $ns_status = $NS->Spawn ();

    if ($ns_status != 0) {
        print STDERR "ERROR: tao_cosnaming returned $ns_status\n";
        exit 1;
    }

    if ($nstarget->WaitForFileTimed ($nsiorbase,
                                     $nstarget->ProcessStartWaitInterval()) == -1) {
        print STDERR "ERROR: cannot find file <$nstarget_nsiorfile>\n";
        $NS->Kill (); $NS->TimedWait (1);
        exit 1;
    }
    if ($nstarget->GetFile ($nsiorbase) == -1) {
        print STDERR "ERROR: cannot retrieve file <$nstarget_nsiorfile>\n";
        $NS->Kill (); $NS->TimedWait (1);
        exit 1;
    }
}

if ($ntarget->PutFile ($nsiorbase) == -1) {
    print STDERR "ERROR: cannot set file <$ntarget_nsiorfile>\n";
    $NS->Kill (); $NS->TimedWait (1);
    exit 1;
}
if ($c1target->PutFile ($nsiorbase) == -1) {
    print STDERR "ERROR: cannot set file <$c1target_nsiorfile>\n";
    $NS->Kill (); $NS->TimedWait (1);
    exit 1;
}
if ($starget->PutFile ($nsiorbase) == -1) {
    print STDERR "ERROR: cannot set file <$starget_nsiorfile>\n";
    $NS->Kill (); $NS->TimedWait (1);
    exit 1;
}
if ($c2target->PutFile ($nsiorbase) == -1) {
    print STDERR "ERROR: cannot set file <$c2target_nsiorfile>\n";
    $NS->Kill (); $NS->TimedWait (1);
    exit 1;
}

$n_status = $N->Spawn ();
if ($n_status != 0) {
    print STDERR "ERROR: notifier returned $n_status\n";
    $NS->Kill (); $NS->TimedWait (1);
    exit 1;
}
sleep $sleeptime;

$c1_status = $C1->Spawn ();
if ($c1_status != 0) {
    print STDERR "ERROR: consumer returned $c1_status\n";
    $NS->Kill (); $NS->TimedWait (1);
    $N->Kill (); $N->TimedWait (1);
    exit 1;
}
#XXX sleep $sleeptime;

$c2_status = $C2->Spawn ();
if ($c2_status != 0) {
    print STDERR "ERROR: consumer returned $c2_status\n";
    $NS->Kill (); $NS->TimedWait (1);
    $N->Kill (); $N->TimedWait (1);
    $C1->Kill (); $C1->TimedWait (1);
    exit 1;
}
#XXX sleep $sleeptime;

$s_status = $S->SpawnWaitKill ($starget->ProcessStopWaitInterval() + 60);
if ($s_status != 0) {
    print STDERR "ERROR: supplier returned $s_status\n";
    $NS->Kill (); $NS->TimedWait (1);
    $N->Kill (); $N->TimedWait (1);
    $C1->Kill (); $C1->TimedWait (1);
    $C2->Kill (); $C2->TimedWait (1);
    exit 1;
}

$c1_status = $C1->TerminateWaitKill ($c1target->ProcessStopWaitInterval());
if ($c1_status != 0) {
    print STDERR "ERROR: consumer 1 returned $c1_status\n";
    $status = 1;
}

$c2_status = $C2->TerminateWaitKill ($c2target->ProcessStopWaitInterval());
if ($c2_status != 0) {
    print STDERR "ERROR: consumer 2 returned $c2_status\n";
    $status = 1;
}

$n_status = $N->TerminateWaitKill ($ntarget->ProcessStopWaitInterval());
if ($n_status != 0) {
    print STDERR "ERROR: notifier returned $n_status\n";
    $status = 1;
}

if ($no_naming_service == 0)
{
    $ns_status = $NS->TerminateWaitKill ($nstarget->ProcessStopWaitInterval());
    if ($ns_status != 0) {
        print STDERR "ERROR: tao_cosnaming returned $ns_status\n";
        $status = 1;
    }
    $nstarget->DeleteFile($nsiorbase);
}

$ntarget->DeleteFile($nsiorbase);
$c1target->DeleteFile($nsiorbase);
$c2target->DeleteFile($nsiorbase);
$starget->DeleteFile($nsiorbase);

exit $status;
