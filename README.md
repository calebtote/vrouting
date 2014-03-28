vrouting
========

Virtual Routing

- This started as a project for class, so some things were implemented for the sake of hacking stuff together by the deadline.

**Things to no judge me for (yet):**
 - Using a global header file
 -  Not using a 3rd party library for tokenizing my strings (and actually, not conforming to any standard tokenizing pattern really)
 - Using sleep() to get around some propogation delays between nodes; with more robust error checking of ACKs, these can easily be taken out
 - Using UDP everywhere; Node-to-Manager connections should really be TCP, but again time was an issue here
 
For known issues that should be resolved, see the issues tab to the right. If you have a suggestion, feel free to create an issue.
