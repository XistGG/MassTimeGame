# Mass Time Game

The purpose of this project is to test time dilation, pausing and resuming
of a UE Mass simulation.

For this project to compile and run, you need to be using the `ue5-main` Unreal Engine.

I set the `EngineAssociation = ""` in the `.uproject` so you can clone this in the
custom Engine source directory.

## Current state of this sample

### Pause/Resume Mass is working

You can currently pause and resume the Mass simulation.
The supporting engine code has been merged into `ue5-main`.

### Time dilation is not currently implemented

I had implemented some Engine changes to support the Mass time dilation,
but after receiving feedback from Epic, that wasn't the direction they intend
to take Mass.

An alternative was presented, and I plan to implement that, it's just not done yet.
(TLDR I'll time dilate the entire game, including Mass, excluding the Player actors).

## Sample PIE Session

- **Ignore the art and animations**, this is a tech demo
- Start PIE
  - click `-` and `+` to change the sim time dilation *(Currently **Work in Progress**)*
  - click `SPACEBAR` to pause/resume
- Game + Character + all other non-Mass actors are *unaffected* by Mass time dilation
