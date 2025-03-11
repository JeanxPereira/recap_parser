<div align="center">
  <img src="res/recap_parser.png" alt="Darkspore Parser" width="256" />
</div>

<h1 align="center">Darkspore Parser</h1>
<p align="center">A simple tool for parsing Darkspore binary files.</p>

## Features  
- Parses various Darkspore file types including `.Phase`, `.CharacterAnimation`, `.AiDefinition`, `.ClassAttributes`, `.NpcAffix`, `.PlayerClass`, `.NonPlayerClass`, `.Noun`, `.MarkerSet`, and `catalog`.  
- Exports parsed data in plain text or XML format. (WIP)
- Supports recursive parsing of directories containing Darkspore files.

## How to Use  
1. Download the latest release from the [Releases page](https://github.com/yourusername/DarksporeParser/releases).  
2. Run the program via the command line:  
   ```bash
   darkspore_parser.exe <path> <filetype> [recursive] [xml]
   ```
   - Replace `<path>` with the file or directory path containing the Darkspore file(s).
   - Replace `<filetype>` with one of the supported file types:  
     `Phase`, `CharacterAnimation`, `AIDefinition`, `ClassAttributes`, `NpcAffix`, `PlayerClass`, `NonPlayerClass`, `Noun`, `MarkerSet`, or `catalog`.
   - Include `recursive` to parse all files in a directory (files must have the given filetype as their extension).
   - Include `xml` to export the parsed output as XML.

## Credits  
- dalkon to make the original parser and your findings on how these formats are interpreted within the game executable