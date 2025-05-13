# Mass Time Game

The purpose of this project is to test time dilation, pausing and resuming
of a UE Mass simulation.

[YouTube video demonstration and code discussion](https://youtu.be/lUWjiRx5LkM) (1.5 minute demo, 18.5 minute discussion)

Compatible UE Version: 5.6

I set the `EngineAssociation = ""` in the `.uproject` so you can clone this into your Custom Engine `Sandbox/`
and it will automatically build with your Custom Engine.

## Sample PIE Session

- Start PIE
  - click `-` and `+` to change the sim time dilation
  - click `SPACEBAR` to pause/resume
- Notice:
  - During Pause state, Mass **is not ticking**, time is standing still as far as Mass is concerned.
  - During Play state, time dilation is accomplished via global time dilation.
  - Player Controller/Character/Widgets/etc are NOT dilated in either Play or Pause state, they always run in real time.
- **Ignore the art and animations**, this is a code/tech demo, I am not an animator.
