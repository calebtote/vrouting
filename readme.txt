Things I didn't have time to implement:
 - STDIN updates for distance vector ( I tried hard though ;_; )

Things to try out:
 - In globals.h, increase the #logging level to see how things are implemented at a more granular level

Things to consider:
 - Please don't judge me for:
 - - having a global.h file
 - - making everything public
 - - using sleep() to hack around propagation delays
 - - not segregating Distance Vector from Routing Node as much as I could have
------ These were time-hacks, and I figured since ANSI C was allowed, these details shouldn't matter for this project.
