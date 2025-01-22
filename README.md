# Mass Time Game

The purpose of this project is to test time dilation, pausing and resuming
of a UE Mass simulation.

For this project to compile and run, you need to be using Xist's custom UE5 engine
(the [`xist-mass-time`](https://github.com/XistGG/UnrealEngine/tree/xist-mass-time) branch)
which is based on Epic's `ue5-main` Github branch.

I set the `EngineAssociation = ""` in the `.uproject` so you can clone this in the
custom Engine source directory.

## Sample PIE Session

- **Ignore the art and animations**, this is a tech demo
- Start PIE
  - click `-` and `+` to change the sim time dilation
  - click `SPACEBAR` to pause/resume
- Game + Character + all other non-Mass actors are *unaffected* by Mass time dilation

[![PIE Session](./Images/PIESession.gif)](./Images/PIESession.gif)

## Which branch is which?

The `main` branch of this game is the latest.
It requires the `xist-mass-time` custom engine branch.
This adds time dilation in addition to play/pause.

| Game Branch                                                          | XistGG UE5 Branch Required                                                     |
|----------------------------------------------------------------------|--------------------------------------------------------------------------------|
| [`main`](https://github.com/XistGG/MassTimeGame/tree/main)           | [`xist-mass-time`](https://github.com/XistGG/UnrealEngine/tree/xist-mass-time) |
| [`xist-mass`](https://github.com/XistGG/MassTimeGame/tree/xist-mass) | [`xist-mass`](https://github.com/XistGG/UnrealEngine/tree/xist-mass)           |

The `xist-mass` game branch contains only the play/pause code,
which is based on the UE5 `xist-mass` branch, as submitted in
[PR#12696](https://github.com/EpicGames/UnrealEngine/pull/12696).

Because that PR has been approved for merging, I've frozen UE5 `xist-mass` and
archived the state of the game test in its own corresponding `xist-mass` branch.
