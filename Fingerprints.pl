
# Copyright 2019 CANARIE Inc. All Rights Reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, 
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, 
#    this list of conditions and the following disclaimer in the documentation 
#    and/or other materials provided with the distribution.
#
# 3. The name of the author may not be used to endorse or promote products 
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY CANARIE Inc. "AS IS" AND 
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# This script prints 10 board IDs corresponding to each of the fingerprints array elements below

my @chars = ('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z');

my @fingerprints = ("0010", "0011", "0100", "0101", "0110", "1001", "1010", "1011", "1100", "1101");

# List of board IDs created so far - used to detect duplicates.
my @IDList = ();

#index into @chars
my $charsIndex = 0;

for my $thePrint (@fingerprints)
{
	my $boardID = "";

	print "Board IDs for fingerprint $thePrint\n";
	
	# we want 10 Board IDs per fingerprint
	for (my $k = 0; $k < 10; $k++)
	{
	    # Flag which, when set, indicates that the board ID we just generated
	    # is a duplicate.
	    my $isOkay = 0;
	    
	    # Save our starting index. If this board ID turns out to be a duplicate, we
	    # will rewind to this point, move to the next element of @chars and try again.
	    my $startingIndex = $charsIndex;
	    while ($isOkay == 0)
	    {
	        ($boardID,$charsIndex) = CreateBoardID($charsIndex, $thePrint, \@chars);
	        
	        # if we have already generated this board ID (due to wrap around)
	        if ( grep( /^$boardID$/, @IDList ))
	        {
	            # print "Duplicate found - " . $boardID . " - Trying again\n";
	            
	            # Go to the element of @chars after the one we started with and try again
	            $charsIndex = $startingIndex + 1;
	            if ($charsIndex >= scalar(@chars))
	            {
	                $charsIndex = 0;
	            }
	            $startingIndex = $charsIndex;
	        }
	        else 
	        {
	            # Not a duplicate, we are done with this board ID
	            $isOkay = 1;
	        }
	    }
	    
	    # Save this ID so we can check for duplicates later
        push @IDList, $boardID;
		print $boardID . "\n";
		
		
	}

	print "\n---------------------------------------------------\n";
}

# -----------------------------------------------------------------
sub AddCheckBytes
{
	my ($theID) = @_;
		
	#Storage for the sum of the mangled ID bytes
	my $mangledSum = 0;
	
	
	# I just made this algorithm up. First, flip the nibbles in each of the ID bytes
	# and add the bytes up
	for (my $i = 0; $i < 4; $i++)
	{
		$mangledSum += FlipNibbles(substr($theID,$i,1));
	}
	
	#Then clamp the result to be between [0 .. 99]
	$mangledSum = $mangledSum % 100;
	
	# Convert to a string
	$theID .= sprintf "%02d", $mangledSum;

	return ($theID);
}
	
# -----------------------------------------------------------------
sub FlipNibbles
{
	my ($theByte) = @_;
	
	my $temp = ord($theByte);
	my $returnValue = ord($theByte);
	
	$temp = ($temp * 16) & 0xf0; $returnValue = int($returnValue / 16);
	
	$returnValue = $returnValue + $temp;
	return $returnValue;
}

# -----------------------------------------------------------------
sub CreateBoardID 
{
    my  ($index, $fingerprint, $charRef)  = @_;
   
    my $theID = "";
    
    my @chars = @{ $charRef };
        
    # for each character in the current fingerprint
	for (my $j = 0; $j < length($fingerprint); $j=$j+1)
	{ 
	    # Extract the next character
	    my $nextChar = substr ($fingerprint, $j,1);
	    
	    # Set $comparator to match the lsb of the current fingerprint character
	    my $comparator;
			
	    if ($nextChar == '0')
	    {
	        $compartor = 0;
	    }
	    else
	    {
	        $comparator = 1;
	    }
			
			
	    # Find the next character in @chars whose lsb matches $comparator
	    my $done = 0;
	    while ($done == 0)
	    {
	        if ((ord(@chars[$index]) & 0x01) == $comparator)
	        {
	            $theID .= @chars[$index];
	            $done = 1;
	        }
	        
	        $index++;
	        if ($index >= scalar(@chars))
	        {
	            $index = 0;
	        }
	    }
	}		
		
    $theID = AddCheckBytes ($theID);
    return ($theID, $index);
}
