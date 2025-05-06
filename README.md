<div align="center">
  <img src="res/recap_parser.png" alt="Darkspore Parser" width="256" />
</div>

<h1 align="center">Darkspore Parser</h1>
<p align="center">A simple tool for parsing Darkspore binary files.</p>

## Filetypes

* `.AffixTuning`
* `.AiDefinition`
* `Catalog (catalog_131)`
* ~~`ChainLevels`~~
* `.CharacterAnimation`
* `.CharacterType`
* `.ClassAttributes`
* `.Condition`
* `.CrystalTuning`
* `.DifficultyTuning`
* `.DirectorTuning`
* `.EliteNPCGlobals`
* `.Level`
* `.LevelConfig`
* `.LevelObjectives`
* `.LootPreferences`
* `.LootPrefix`
* `.LootRigblock`
* `.LootSuffix`
* `.MagicNumbers`
* `.MarkerSet`
* `.NavPowerTuning`
* `.NonPlayerClass`
* `.Noun`
* `.NpcAffix`
* `.ObjectExtents`
* `.Phase`
* `.PlayerClass`
* ~~`.PVPLevels`~~
* `.SectionConfig`
* `.ServerEventDef`
* `.SpaceshipTuning`
* `.TestAsset`
* `.UnlocksTuning`
* `.WeaponTuning`


## Features
- Parses various Darkspore file types including
- Exports parsed data in plain text or XML format. (WIP)
- Supports recursive parsing of directories containing Darkspore files.

## How to Use  
1. Download the latest release from the [Releases page](https://github.com/yourusername/DarksporeParser/releases).  
2. Run the program via the command line:  
   ```bash
   darkspore_parser.exe <path> <filetype> [recursive] [xml]
   ```
   - Replace `<path>` with the file or directory path containing the Darkspore file(s).
   - Replace `<filetype>` with one of the supported [filetypes](https://github.com/JeanxPereira/recap_parser?tab=readme-ov-file#filetypes)
   - Include `recursive` to parse all files in a directory (files must have the given filetype as their extension).
   - Include `xml` to export the parsed output as XML.

## Credits  
- dalkon to make the original parser and your findings on how these formats are interpreted within the game executable
