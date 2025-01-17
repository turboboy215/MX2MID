# MX2MID
MusyX (GBC) to MIDI converter

This tool converts music from Game Boy Color games using the MusyX sound engine (by Factor 5) to MIDI format.

It works with ROM images. To use it, you must specify the name of the ROM followed by the number of the main bank containing the sound data (in hex).
In addition to converting MIDI files, this tool also includes a mode to rip samples from games that use PCM samples or waveforms to WAV format. This mode can be used by using the parameter "S" at the end. The parameter "M" at the end is used to extract music; however, this is optional.

Examples:
* MX2MID "Uno (E) (M6) [C][!].gbc" 2 M
* MX2MID "Shrek - Fairy Tale Freakdown (U) (M6) [C][!].gbc" 66 M
* MX2MID "Harry Potter and The Sorcerer's Stone (UE) (M13) [C][!].gbc" 10 M

Although there already exists Python tools (https://github.com/GauChoob/musyx) that can extract MIDIs and samples from games using this sound engine, it is very difficult to get working, and is likely more suited to replacing/hacking audio data. Using the information specified at the link, I was able to create my own converter which is much easier to use. Also included, as usual, is a "prototype" converter, MX2TXT.

Supported games:
  * Alone in the Dark: The New Nightmare
  * Armada: FX Racers
  * Armorines: Project S.W.A.R.M.
  * Army Men: Sarge's Heroes 2
  * Barbie: Fashion Pack Games
  * Barbie: Pet Rescue
  * Bob the Builder: Fix It Fun!
  * Buffy the Vampire Slayer
  * Championship Motocross 2001 Featuring Ricky Carmichael
  * CyberTiger
  * Denki Blocks!
  * Dragon Tales: Dragon Adventures
  * DynaMike (prototype)
  * F1 Championship Season 2000
  * Gift
  * Harry Potter and the Chamber of Secrets
  * Harry Potter and the Philosopher's Stone
  * Hercules: The Legendary Journeys
  * Heroes of Might and Magic II
  * Hoyle Casino
  * Indiana Jones and the Infernal Machine
  * Janosch: Das große Panama-Spiel
  * Magi Nation
  * Magi Nation: Keeper's Quest
  * Mat Hoffman's Pro BMX
  * Maya the Bee: Garden Adventures
  * Mission Bravo
  * MTV Sports: T.J. Lavin's Ultimate BMX
  * The Mummy Returns
  * Noddy and the Birthday Party
  * Portal Runner
  * Pumuckl's Abenteuer bei den Piraten
  * Pumuckl's Abenteuer im Geisterschloss
  * Road Champs: BXS Stunt Biking
  * Robot Wars: Metal Mayhem
  * Rocket Power: Gettin' Air
  * Rocky Mountain Trophy Hunter
  * Rugrats: Totally Angelica
  * Sabrina: The Animated Series: Spooked!
  * Sabrina: The Animated Series: Zapped!
  * San Francisco 2049
  * Shrek: Fairy Tale Freakdown
  * Star Wars Episode I: Obi-Wan's Adventures
  * Stuart Little: The Journey Home
  * Tech Deck Skateboarding
  * Test Drive 2001
  * Toy Story Racer
  * Tweenies: Doodles' Bones
  * Tyro RC: Racin' Ratz
  * Uno
  * Walt Disney World Quest: Magical Racing Tour
  * Wendy: Every Witch Way
  * WWF Betrayal
  * Xena: Warrior Princess
  * Xtreme Sports

## To do:
  * GBS file support
