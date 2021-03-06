#!/usr/bin/env perl

use strict;
use warnings;

use FindSeed ();

my $x = <<'EOF';
EOF

my $old = <<'EOF';
  4022	7556
 20358	7566
 28825	7566
 19763	7568
 23774	7573
 31119	7609
 21491	7610
 29866	7662
  4609	7699
 18320	7708
 14269	7754
 24341	7810
 23424	7871
 12957	8015
 20785	8018
  8604	8127
 16768	8628
 16837	9002
 28920	9004
 13765	9072
  8858	10379
 13304	10431
 25599	10831
 27188	10839
 25315	10870
  6182	11241
 17323	12701
 24106	13720
 17355	14225
 15592	15024
 31302	15679
  1941	20200
EOF

my @deals = ( <<'EOF' =~ /^ *([0-9]+)/gms );
 30359	7065
  4808	7083
  1734	7234
 12056	7238
 10692	7317
   753	7318
 31465	7334
  2306	7341
  5376	7596
 19837	7628
 20358	7740
 28825	7740
 23774	7747
 21491	7784
 29866	7836
  4609	7873
 24341	7984
 23424	8045
  8604	8301
 16768	8802
 13765	9246
  8858	10553
 13304	10605
 25599	11005
 27188	11013
 25315	11044
  6182	11415
 24106	13894
 17355	14399
 15592	15198
 31302	15853
  1941	20374
EOF

my @scans = (
    q#--method random-dfs -to "01[2345789]"#,
    q#--method random-dfs -to "01[234579]"#,
    q#--method random-dfs -to "01[234589]"#,
    q#--method random-dfs -to "01[234567]" -dto2 "6,0132[456789]"#,
    q#--method random-dfs -to "01[234567]" -dto2 "5,01[2345789]"#,
    q#--method random-dfs -to "[01][23457]"#,
    q#--method random-dfs -to "[0123457]"#,
    q#--method random-dfs -to "[01][23457]" -dto2 "7,[0123][456789]"#,
    q#--method random-dfs -to "[01][23457]" -dto2 "7,[0123][4567]"#,
    q#--method random-dfs -to "[01][23457]" -dto2 "5,[0123][4567]"#,
    q#--method random-dfs -to "[01][23457]" -dto2 "10,[0123][4567]"#,
    q#--method random-dfs -to "[01][23457]" -dto2 "8,[0123][4567]"#,
q#--method random-dfs -to "01[234567]" -dto2 "5,01[2345789]" -dto2 "10,[0123][4567]"#,
);

FindSeed->parallel_find(
    {
        scan      => \@scans,
        deals     => \@deals,
        threshold => 10,
    },
);
