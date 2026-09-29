# Approved grenade and rifle special-shot audio

User-approved on 2026-09-29. These two SoundWave assets do not replace ordinary
weapon fire sounds. Gameplay/BP assignment is a separate step.
Both sources below were verified as CC0 on their primary source pages.
License: https://creativecommons.org/publicdomain/zero/1.0/

## S_Grenade_Explosion_Compact_v2

- Author: unfa.
- Source: https://freesound.org/people/unfa/sounds/609587/
- Title: Grenade Explosion SFX (medium-sized, meaty, realistic).
- The author describes firecracker recordings and Vitalium synthesis, not an
  actual grenade field recording. Input was the public HQ MP3 preview, not FLAC.
- Processing: 48 kHz stereo PCM16; 55 Hz highpass, 2.2 kHz +1.5 dB,
  sustained sub-200 Hz body reduced after 100 ms (220 ms decay, 28% floor),
  peak normalization and final quiet 100 ms fade. Full 5.316292 s retained.
- Approved WAV SHA-256:
  ff3f44a0378f5c15a0eac423872e5efd1a9d2049bae06d92e4f7ce409e6e77a7

## S_Rifle_SpecialShot_Piercing_v2

- Source: The Free Firearm Sound Library.
- Authors: Ben Jaszczak, Brian Nelson, Kevin Heras, Matthew Nanney.
- https://opengameart.org/content/the-free-firearm-sound-library
- Input: Tikka/W_29P.wav near single shot, Tikka/W_24P.wav mid-distance decay.
- Processing: first isolated near shot; 38 Hz highpass, body/attack EQ,
  short low-frequency layer, 140-260 ms crossfade into recorded mid-distance
  decay, gentle tanh crest shaping, low-level 115 ms designed air transient
  starting at 16 ms, peak normalization and quiet end fade. Duration 2.9 s.
- Designed composite, not an unchanged recording or a recording of the exact
  in-game rifle. No second shot, charge sound, pitch shift, or synthetic echo.
- Approved WAV SHA-256:
  5decd6629df151ece16d557f4d77d80e885519937f05268182c9556324fc3959

The audition-only Listen_* WAV files include 200 ms leading silence and are not
the game sources. Only the two unpadded approved WAVs are imported.
