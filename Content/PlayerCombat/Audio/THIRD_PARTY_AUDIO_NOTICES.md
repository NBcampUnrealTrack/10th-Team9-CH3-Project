# Third-party player weapon audio

## Michel Baradari — Chaingun, pistol, rifle, shotgun shots

Sounds (c) Michel Baradari — https://apollo-music.de/

Source: https://opengameart.org/node/2566

License: Creative Commons Attribution 3.0 Unported (CC BY 3.0)
https://creativecommons.org/licenses/by/3.0/

Original files used:

- `shots/cg1.wav` → `/Game/PlayerCombat/Audio/LibraryShots/S_FP_Rifle_Fire_Library` (retained earlier version; no longer selected by BP_Rifle after RecordedV2)
- `shots/shotgun.wav` → `/Game/PlayerCombat/Audio/LibraryShots/S_FP_Shotgun_Fire_Library`
- `shots/rifle.wav` → `/Game/PlayerCombat/Audio/LibraryShots/S_FP_Sniper_Fire_Library`

Changes: gain balancing and short boundary fades; original stereo channels, sample rate and full shot/mechanical tails retained. These are existing designed game sound effects, not new recordings of the exact displayed weapon models. No endorsement by the author is implied.

Include this attribution, source link, license link, and modification notice with any distributed game using these assets (for example in game credits or an accompanying third-party notices file). Unreal does not automatically package this Markdown file as an in-game credit.

## Player pistol

The player pistol references the pre-existing project asset `/Game/Colleague/Audio/S_Hayakawa_Pistol_Fire`, as requested. This change does not alter the companion sound asset, or establish a new license for that pre-existing recording. Preserve its existing provenance/licensing records.

## RecordedV2 — rifle shot

Source: The Free Firearm Sound Library, by Ben Jaszczak, Brian Nelson, Kevin Heras and Matthew Nanney.
https://opengameart.org/content/the-free-firearm-sound-library

License: CC0 1.0 — https://creativecommons.org/publicdomain/zero/1.0/

`Prepared SFX Library/AR-15/D_32P.wav`, first near-distance AR-15 shot, becomes
`/Game/PlayerCombat/Audio/RecordedV2/S_FP_Rifle_Fire_Recorded_v2`.
Changes: extracted 0.69–1.69 seconds (the second shot is excluded), stereo-to-mono downmix,
96 kHz to 48 kHz resampling, 16-bit PCM, gain adjustment and short fades.
This is an AR-15 recording, not a claim of a recording of the exact displayed rifle.

## RecordedV2 — reload handling

SpringySpringo, Gun reload sounds:
https://opengameart.org/content/gun-reload-sounds

License: CC0 1.0 — https://creativecommons.org/publicdomain/zero/1.0/

The author describes these as **airsoft** recordings:
`gunreload1.wav`, `assaultriflereload1.wav`, `shotguncock.wav`.

- `S_FP_Pistol_Reload_Recorded_v2`: generic reload recording, padded to 1.6 seconds.
- `S_FP_Rifle_Reload_Recorded_v2`: assault-rifle reload recording, event spacing adjusted to 1.8 seconds.
- `S_FP_Shotgun_Reload_Recorded_v2`: edited handling clicks from `gunreload1.wav`, followed by the complete `shotguncock.wav` pump stroke; 1.8-second designed sequence, not a recording of a complete shell-loading cycle.
- `S_FP_Sniper_Reload_Recorded_v2`: opening handling click from `gunreload1.wav`, followed by the Danwardvs bolt action below; 1.8 seconds.

All four assets are under `/Game/PlayerCombat/Audio/RecordedV2/`.
Changes: selected event cuts and spacing, mono downmix, 48 kHz/16-bit PCM conversion,
gain balancing and boundary fades. No pitch shift. The recordings are not specific to
the displayed weapon models, and current gameplay reload durations are unchanged.

## RecordedV2 — sniper bolt action

Danwardvs, `22 Bolt.wav` (Cooey 600 .22 rifle bolt action):
https://freesound.org/people/Danwardvs/sounds/204204/

License: CC0 1.0 — https://creativecommons.org/publicdomain/zero/1.0/

The public high-quality MP3 preview was used, not the login-only original WAV.
Decoded and resampled to 48 kHz/16-bit PCM; used after the opening handling sound in
`S_FP_Sniper_Reload_Recorded_v2`, with gain/fades and timing adjustment, without pitch shift.

## Preserved assets

Old procedural reload assets and the earlier rifle shot remain on disk for recovery,
but the four player weapon BPs now select the RecordedV2 reload sounds.
Pistol/Hayakawa, shotgun and sniper fire sounds and existing dry-fire references are unchanged.

## M4Thump — selected rifle shot revision

mnslugger20, `M4 Assault rifle firing.wav`:
https://freesound.org/people/mnslugger20/sounds/259758/

License: CC0 1.0 — https://creativecommons.org/publicdomain/zero/1.0/

The public HQ MP3 preview (not the login-only original WAV) was decoded to PCM.
The author describes a recorded .223 shot with added bass, shell-like silverware sounds
and echo. It is a designed effect, not an unprocessed recording of the exact in-game gun.

`/Game/PlayerCombat/Audio/M4Thump/S_FP_Rifle_Fire_M4_Thump_v3` uses the last attack/decay
at 9.938–10.360 seconds, excluding subsequent attacks; some previous-burst ambience remains.
Changes: 48 kHz stereo PCM16; 2 ms onset / 45 ms end fades; 40 Hz high-pass;
170 Hz +3 dB and 380 Hz +1.2 dB bell EQ; 3.8 kHz high shelf -3.5 dB;
11 kHz low-pass; peak balancing. No pitch shift or gameplay fire-rate change.
The edited single shot is 0.422 seconds long. Earlier AR-15 RecordedV2 remains on disk
for recovery. Reload sound assignments remain RecordedV2.

## FireBalance — player-only volume adjustment (2026-09-28)

- `S_FP_Pistol_Fire_Balanced_v4` duplicates the project's existing
  `/Game/Colleague/Audio/S_Hayakawa_Pistol_Fire`. Playback volume is 0.8 times
  the source's previous value. This does not establish a new license for that
  pre-existing asset. The shared Hayakawa source remains unchanged.
- `S_FP_Rifle_Fire_M4_Balanced_v4` duplicates the M4Thump v3 asset above,
  retaining its CC0 source and processing history. Playback volume is 1.3 times
  the source's previous value.

Both assets are under `/Game/PlayerCombat/Audio/FireBalance`. Source PCM, pitch,
duration and original sound assets are unchanged; only duplicate SoundWave
playback volume and the two player weapon FireSound references were changed.
Reload sounds and the common weapon FeedbackVolume values remain unchanged.
