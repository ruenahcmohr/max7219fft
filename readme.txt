The answer is:

make
make arduinoinstall0

problem? see the makefile!


So, back in 1967 there was a hack of two peoples methods to do
fast fourier transforms. It was written in fortran, which could
not do binary operations, and could only handle one data array.
This got hacked into a BASIC version, the coders did not know
how it worked, just that it did, and, less the limits of BASIC
they ported it 1:1. Then someone ported it to C, again, not 
knowing how it worked, ported it 1:1, with nonally the fortran
quirks, but the added basic quirks. This has since been ported 
to java, again, 1:1.

I had ONE HELL of an adventure tracing back the code to its origins
figuring out what it was doing, and rewriting it.

Contained here, is a little challange to myself, an 8 bit integer 
version. The code that this is sourced from can be floating point
or whatever, but this is about the minumum this code can be, the 
numbers *just* barely fit in 8 bits.

Its really exciting to write something like this and have the 
real-world results come out *bang on* for the math predictions.
( The centre frequency of the bins is dead on )

I'v included the origional fortran source as a reference.
I think there is 1 typo in a comment, I cant remember if it was
an origional typo I carried forward for authenticity.

  Play hard! 
    Rue.
