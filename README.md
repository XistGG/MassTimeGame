# Mass Time Game

The purpose of this project is to test time dilation, pausing and resuming
of a UE Mass simulation.

For this project to compile and run, you need to be using UE 5.6+
*(or if 5.6 is not yet released, use `ue5-main`)*.

I set the `EngineAssociation = ""` in the `.uproject` so you can clone this in the
custom Engine source directory and it will compile without requiring any changes.

## Sample PIE Session

- Start PIE
  - click `-` and `+` to change the sim time dilation
  - click `SPACEBAR` to pause/resume
- Notice:
  - During Pause state, Mass **is not ticking**, time is standing still as far as Mass is concerned.
  - During Play state, time dilation is accomplished via global time dilation.
  - Player Controller/Character/Widgets/etc are NOT dilated in either Play or Pause state, they always run in real time.
- **Ignore the art and animations**, this is a code/tech demo, I am not an animator.
