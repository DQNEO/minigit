package MyLib;
use base 'Exporter';
our @EXPORT_OK = qw/slurp/;

sub slurp {
    my $filename = shift;
    local $/;
    open my $fh, '<', $filename;
    return <$fh>;
}

1;
