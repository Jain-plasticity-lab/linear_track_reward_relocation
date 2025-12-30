# Overview
Microcontroller code to enable automated behavior setup with TTL I/O for synchronisation and communication with external neural data acquisition setups. Currently configured to work with RWD 810 single channel fiber photometry system with acquisition locked input trigger for behavior session and defined duration dependent timeout output trigger to stop fiber photometry acquisition.

# Setup
 - Ensure all files are under a parent dir with name 'linear_track_reward_relocation'
 - Define connections to and from the microcontroller under config.h
 - Adjust required timing parameters in config.h
 - Compile and load onto a compatible arduino with connections as defined in config.h
 - Optional input trigger from a neural data acquisition system - start can be defined by the rising edge or a complete square pulse.
 - Output trigger pulse sent following completion of runtime execution
 - Event trigger outputs provided for inputs to the neural data acquisition setup
 - Event logs are encoded and communicated over Serial Communication Port (COM) along with their timmestamps - can be saved using logging tools like Putty - listen on the connected COM port with the same baud rate as definied under config.h

# TODO:
 - [ ] Refactor existing code with class abstraction
 - [ ] Modularize and enable arbitrary length sequence rule definition for reward
   - Sequence completion track as increment in pointer location over the reward sequence array
