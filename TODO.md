## TODO file for the BeeBit device component.

- [x] Use a neural net to identify people
- [x] Utilise a CSRT tracker to identify and track people as they move
- [x] Fix KCF tracker
- [x] Make centroid tracker parameters configurable
- [x] Allow change of tracker type in configuration
- [x] Track objects over a line
- [ ] Move entire tracking subroutine over to a different thread
- [ ] Make configurable GUI for testing
- [ ] Multithread trackers
- [ ] Detect people inside a box in addition to line detection
- [ ] Abstract detection away so that PeopleCounter::performDetection() performs a detection rather than requiring a main loop.
- [ ] Construct daemon that communicates with the server software