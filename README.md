<div align="center">
  <img src="res/recap_parser.png" alt="Darkspore Parser" width="256" />
</div>

<h1 align="center">Darkspore Parser</h1>
<p align="center">A simple tool for parsing Darkspore binary files.</p>

## Features
- Parses various Darkspore file types
- Exports parsed data in XML or YAML format
- Supports recursive parsing of directories containing Darkspore files

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
* `.LootSuffix` `[5.3.0.103 / 5.3.0.127]` 
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

## How to Use  

### Basic Usage
```bash
recap_parser [options] <file|directory>
```

### Command Line Options
- `--help, -h` - Show help message
- `--xml` - Export to XML format
- `--yaml, -y` - Export to YAML format
- `--silent` - Removes all logs, except error logs
- `--debug, -d` - Enable debug mode to show offsets
- `--recursive, -r [extension]` - Process all supported files recursively. Optionally filter by extension
- `--output, -o <directory>` - Specify output directory for exported files
- `--log, -l` - Export complete log to a text file
- `--sort-ext, -s` - : Organize output files in subdirectories by file extension
- `--game-version` - : Specify game version (5.3.0.103, 5.3.0.127)

### Examples

#### Parse a single file:
```bash
recap_parser file.noun --xml -o output/
```

#### Parse all supported files in a directory recursively:
```bash
recap_parser --recursive --xml -o ./output/ ./AssetData_Binary/
```

#### Parse and organize by extension (your use case):
```bash
recap_parser --recursive --xml --sort-ext -o ./AssetData_Binary_Parsed ./AssetData_Binary
# Or using the short alias:
recap_parser -r --xml -s -o ./AssetData_Binary_Parsed ./AssetData_Binary
```

This will create a structure like:
```
AssetData_Binary_Parsed/
├── aidefinition/
│   ├── file1.AiDefinition.xml
│   └── file2.AiDefinition.xml
├── noun/
│   ├── creature1.Noun.xml
│   └── creature2.Noun.xml
├── level/
│   ├── level1.Level.xml
│   └── level2.Level.xml
├── nonplayerclass/
│   └── npc1.NonPlayerClass.xml
├── npcaffix/
│   └── affix1.NpcAffix.xml
├── phase/
│   └── phase1.Phase.xml
└── playerclass/
    └── class1.PlayerClass.xml
```

#### Filter by specific extension:
```bash
recap_parser --recursive .noun --xml -o ./output/ ./AssetData_Binary/
```

## Output Organization

### Default Behavior
By default, all exported files are placed directly in the output directory with their original extension preserved:
```
output/
├── creature1.Noun.xml
├── level1.Level.xml
├── npc1.NonPlayerClass.xml
└── ...
```

### With `--sort-ext`
When using `--sort-ext`, files are automatically organized into subdirectories based on their extension:
- Extension names are converted to lowercase
- The leading dot is removed from the extension
- Files without extensions go to an "others" folder

**Benefits:**
- Better organization for large datasets
- Easier to find specific file types
- Cleaner directory structure
- Perfect for processing entire game data directories

## Installation
1. Download the latest release from the [Releases page](https://github.com/yourusername/recap_parser/releases)
2. Extract the executable to your desired location
3. Run from command line as shown in the examples above

## Building from Source
```bash
# Using CMake (Linux/macOS)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

# Using Visual Studio (Windows)
# Open recap_parser.sln in Visual Studio and build
```

## Credits  
- dalkon for the original parser and findings on how these formats are interpreted within the game executable