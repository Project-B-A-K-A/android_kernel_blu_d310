#!usr/bin/perl
my $configFile = $ARGV[0];
my $optrConfigFile = $ARGV[1];
die "the file $configFile is NOT exsit\n" if ( ! -e $configFile);
die "the file $optrConfigFile is NOT exsit\n" if ( (! -e $optrConfigFile) && ($optrConfigFile ne "") );
open FILE, "<$configFile";
my %config_for;
while (<FILE>) {
	if (/^(\w+)\s*=\s*(\w+)/) {
		$config_for{$1} = $2;
	}
}
close FILE;
open FILE, "<$optrConfigFile";
while (<FILE>) {
        if (/^(\w+)\s*=\s*(\w+)/) {
                $config_for{$1} = $2;
        }
}
close FILE;


my $filedir = "mediatek/frameworks/common/src/com/mediatek/common/featureoption";
my $write_filename = "$filedir/FeatureOption.java";
my $input_file = "mediatek/build/tools/javaoption.pm";
open(INPUT,$input_file) or die "can not open $input_file:$!\n";
while(<INPUT>)
{
	chomp;
	next if(/^\#/);
	next if(/^\s*$/);
	push (@need_options,$1) if(/\s*(\w+)\s*/);
}


system("chmod u+w $write_filename") if (-e $write_filename);
system("mkdir -p $filedir") if ( ! -d "$filedir");
die "can NOT open $write_filename:$!" if ( ! open OUT_FILE, ">$write_filename");
print OUT_FILE "\/* generated by mediatek *\/\n\n";
print OUT_FILE "package com.mediatek.common.featureoption;\n";
print OUT_FILE "\npublic final class FeatureOption\n{\n";
foreach my $option (@need_options) {
	if ($config_for{$option} eq "yes") {
		&gen_java_file ($write_filename, $option, "true");
	}
	elsif ($config_for{$option} eq "no"){
		&gen_java_file ($write_filename, $option, "false");
	}
	else
	{
		# add by lixiaobin 20130322 start
		if ($option eq "RGK_IS_WIFISOUND") {
			&gen_java_file ($write_filename, $option, "false");
		}
		# add by lixiaobin 20130322 end
                # add by songqingming 20130402 start
		elsif ($option eq "MTK_IP_DIAL") {
			&gen_java_file ($write_filename, $option, "true");
		}
		# add by songqingming 20130402 end
		print "\"$option\" not match\n";
	}
}
print OUT_FILE "}\n";
close OUT_FILE;
sub gen_java_file {
	my ($filename, $option, $value) = @_;
	print OUT_FILE "    /**\n     * check if $option is turned on or not\n     */\n";
	if ( $option eq "GEMINI") {
		print OUT_FILE "    public static final boolean MTK_${option}_SUPPORT = $value;\n";
		print "public static final boolean MTK_${option}_SUPPORT = $value\n";
	}
	else {
		print OUT_FILE "    public static final boolean ${option} = $value;\n";
		print "public static final boolean ${option} = $value\n";
	}
}
