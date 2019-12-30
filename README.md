# ![LOCKJAW](docs/ljlogo192.png) About

*   [Home](http://www.pineight.com/lj/)
*   About
*   [Download](http://www.pineight.com/lj/dl)

<div id="content">

[Installing](#installing) | [Skinning](#skin) | [Controls](#controls) | [Scoring](#scoring) | [Gimmicks](#gimmicks) | [Options](#options) | [Discussion](#discussion) | [Legal](#legal)

LOCKJAW is a [free software](http://www.gnu.org/philosophy/free-sw.html) implementation of the the Soviet Mind Game, a highly popular computer puzzle game that involves guiding tetrominoes into neat stacks in a well. This game was designed in the mid-1980s by Russian game designer Alexey Pajitnov and was first implemented in a software product called [TETRIS](http://en.wikipedia.org/wiki/Tetris)®.[*](#trademarks) Other products implementing the Soviet Mind Game include [Quadra](http://quadra.sourceforge.net/), [Abandoned Bricks](http://abrick.sourceforge.net/), [KSirtet](http://ksirtet.sourceforge.net/), [F.B.G.](http://fbg.sourceforge.net/), [N-Blox](http://www.neave.com/games/nblox/), and [Heboris](http://www.tetrisconcept.com/forum/viewtopic.php?t=54). While originally developed to parody the behavior of a few notorious implementations of the Soviet Mind Game, LOCKJAW is now a platform for research into the properties of the game, into the effects of rule variations, and into the capabilities of the human mind to react.

[Tetrominoes](http://en.wikipedia.org/wiki/Tetromino) are geometric shapes made of four connected square blocks. There are seven distinct tetrominoes, shaped roughly like letters of the Latin alphabet:

![(Illustration of the seven tetrominoes)](docs/ijlo-stz.png)  
Tetrominoes. Top row: I, J, L, O. Bottom row: S, T, Z.

Some products spell the term as "tetramino", "tetrimino", or "tetrad".

<div class="screenshot">![(Game screenshot)](docs/ljsnap018.png)

LOCKJAW gameplay screen. Hover over objects to see their names.

<map name="ljsnap018_map"><area shape="rect" coords="12,12,48,36" alt="Hold piece" title="Hold piece" style="border: 1px solid white"> <area shape="rect" coords="48,0,192,36" alt="Status" title="Status"> <area shape="rect" coords="96,48,132,108" alt="Falling piece" title="Falling piece"> <area shape="rect" coords="96,168,132,216" alt="Shadow" title="Shadow"> <area shape="rect" coords="12,36,132,276" alt="Well" title="Well"> <area shape="rect" coords="36,276,108,300" alt="Game mode" title="Game mode"> <area shape="rect" coords="144,36,192,252" alt="Next pieces" title="Next pieces"> <area shape="rect" coords="216,60,360,240" alt="Gus" title="Gus"></map> </div>

The well is 10 blocks wide by 20 rows high. (There are four out-of-bounds rows above the top of the visible portion for a total of 24 rows.) The player can rotate and shift the tetrominoes as they fall in order to pack them tightly into the well. If a tetromino lands on the floor or other blocks and remains motionless for half a second, it locks into place, and the next tetromino begins to fall. The next few tetrominoes to fall are displayed in a queue to the right of the playfield. At any time, the player can swap the falling tetromino with the one in the hold box above the playfield, but a tetromino that has been swapped out cannot be immediately swapped back in. The gray blocks below the falling tetromino are the shadow, which shows where the tetromino will land.

A "line", or a complete row of 10 blocks across the well, will disappear, and everything above it moves down by one row. But if the well fills so high that a tetromino is placed entirely out of bounds or in a position such that the next tetromino does not have room to enter, the player "tops out" and the game is over.

## <a name="installing">Installing</a>

The executable included with the official distribution is designed for Microsoft Windows systems. Unzip it into a folder on your hard disk or USB memory card.

The files `lj.exe`, `lj.dat`, `ljblocks.bmp`, and `alleg42.dll` are required to play. (If you get an error about `alleg42.dll`, see the instructions on the LOCKJAW download page.) To start the game, run `lj.exe` in Windows Explorer.

The program should work on any PC running Microsoft Windows 98 or newer operating system with an 800x600 pixel display at 16-bit or greater color depth and DirectX 7 software installed. Its source code is portable to any platform that supports the [Allegro library](http://alleg.sourceforge.net/), but it has been tested only on Microsoft Windows because the author has access only to machines that run Microsoft Windows. If you manage to get it working on any other operating system, get in touch with the author.

To recompile the program, such as if you are testing a patch or porting it to another system:

1.  Install a GCC toolchain, such as MinGW and MSYS from [MinGW.org](http://www.mingw.org/MinGWiki/index.php/GettingStarted)
2.  Install Allegro 4.2.0 from [SourceForge.net](http://alleg.sourceforge.net/)
3.  Install JPGalleg 2.5 from [Enhanced Creations++](http://www.ecplusplus.com/index.php?page=projects&pid=1)
4.  Install DUMB 0.9.3 from [SourceForge.net](http://dumb.sourceforge.net/)  
    (Editor's note: DUMB has a dumb license, but section 6 allows relicensing under GNU GPL. Update: It appears [Debian got the author to relicense it zlib style](http://packages.debian.org/changelogs/pool/main/libd/libdumb/libdumb_0.9.3-5/libaldmb1.copyright).)
5.  Open a command prompt and <kbd>cd</kbd> to the folder containing the file `makefile`
6.  Type `make`

## <a name="skin">Skinning</a>

You can customize the appearance of LOCKJAW for PC by using Notepad or any other text editor to create a skin description that lists the images that shall be used. Images can be in Windows bitmap (`.bmp`), PC-Paintbrush (`.pcx`), Truevision TGA (`.tga`), or JFIF/JPEG (`.jpg`) format. All color depths should be supported. By default, this skin description should be placed in a file called `skin.ini`, in the same folder as `lj.ini`. (You can create and edit .ini files in Notepad or any other plain text editor.) LOCKJAW recognizes the following commands in a skin description:

<dl>

<dt>`ljblocksSRS=_image_`</dt>

<dd>A 192x96 or 192x144 pixel image with 24x24 pixel block images, used in bounding-box rotation systems (SRS and TOD M4). Rows 1 and 2 are for blocks in the well, and rows 3 and 4 are for the falling piece. If rows 5 and 6 are present, the shadow will use those; otherwise, it will use rows 3 and 4.</dd>

<dt>`ljblocksSega=_image_`</dt>

<dd>Same as `ljblocksSRS`, but used in other rotation systems.</dd>

<dt>`ljconnSRS=_image_`</dt>

<dd>A 192x192 pixel image with O tetrominoes of each color, which the loader cuts up to form tetrominoes with the blocks drawn connected, used for blocks in the well and the falling piece (not the shadow or empty areas of the well) in bounding-box rotation systems. If this file is not present, `ljblocksSRS` will be used instead.</dd>

<dt>`ljconnSega=_image_`</dt>

<dd>Same as `ljconnSRS`, but used in other rotation systems. If this file is not present, `ljblocksSega` will be used instead.</dd>

<dt>`color=_#RRGGBB_`</dt>

<dd>A 3- or 6-digit hexadecimal color (e.g. `#ABC` or `#D0FFE3`) for text in the menus and during the game. If not present, the game will use black.</dd>

<dt>`bgcolor=_#RRGGBB_`</dt>

<dd>A color for the menus' background. If not present, the game will use white.</dd>

<dt>`hilitecolor=_#RRGGBB_`</dt>

<dd>A color for the background of highlighted text. If not present, the game will use pale yellow (#FFC).</dd>

<dt>`pfcolor=_#RRGGBB_`</dt>

<dd>A color for text in front of the playfield. If not present, the game will use white.</dd>

<dt>`pfbgcolor=_#RRGGBB_`</dt>

<dd>An "average" color for the playfield, to be displayed behind the pause screen and during the game over animation. If not present, the game will use black.</dd>

<dt>`ljbg=_#RRGGBB_`</dt>

<dd>An 800x600 pixel image to be displayed behind the game. If this file is not 800x600 pixels, the image will be resized (sloppily) after being loaded. If this file is not present, the game will use a plain backdrop of the same color as `bgcolor`.</dd>

<dt>`bgm=_music file_`</dt>

<dd>A music file in Vorbis format (`.ogg`) or tracker format (`.mod`, `.s3m`, `.xm`, or `.it`) to be played during the game. You can create Vorbis format files by extracting audio from your CDs to `.wav` format using [CDex software](http://sourceforge.net/projects/cdexos/) and converting them with [OggDropXPd software](http://www.rarewares.org/ogg.html). You can download tracker format files from [The Mod Archive](http://www.modarchive.com/) or create them yourself using the [OpenMPT music editor](http://lpchip.com/modplug/viewtopic.php?t=18), the continuation of MODPlug Tracker. If this file is not present, the game will not play music.</dd>

</dl>

You can create more than one skin description file and then switch among them by using the following command in `lj.ini`:

<dl>

<dt>`Skin=_skin description file_`</dt>

<dd>A skin description file. If the skin is not present, the game uses the default file names (`ljblocks.bmp`, `ljblocks-sega.bmp`, `ljconn.bmp`, `ljconn-sega.bmp`, `ljbg.jpg`, and `bgm.s3m`).</dd>

</dl>

Example image and sound files are located in `lj-contrib.zip`, available from the [Skins section of the download page](http://www.pineight.com/lj/dl#contrib). You can also customize the sound effects by using [Allegro Grabber software](http://www.allegro.cc/depot/Grabber/) to edit `lj.dat`.

## <a name="controls">Controls</a>

### PC

The default controls in LOCKJAW for PC are as follows:

<dl>

<dt>← Shift left</dt>

<dd>Left arrow key</dd>

<dt>→ Shift right</dt>

<dd>Right arrow key</dd>

<dt>↓ Soft drop</dt>

<dd>Down arrow key</dd>

<dt><u>↓</u> Hard drop</dt>

<dd>Up arrow key</dd>

<dt>↰ Rotate left</dt>

<dd>Z, C</dd>

<dt>↱ Rotate right</dt>

<dd>X</dd>

<dt>↶ Rotate twice</dt>

<dd>W</dd>

<dt>↖ Hold piece</dt>

<dd>S</dd>

<dt>⇤ Shift far left</dt>

<dd>Q</dd>

<dt>⇥ Shift far right</dt>

<dd>E</dd>

<dt><u>↓</u> Firm drop</dt>

<dd>Enter</dd>

<dt>↖ Alt. hold piece</dt>

<dd>D</dd>

</dl>

Controls are configurable to the keyboard or any compatible joystick. (The key labeled "Item" is not yet used in the game.) From the main menu, choose "Game Keys..." and then press the keys in order as prompted. If you don't want to bind a key or button to a given function, press a key that you won't use. The key bindings are saved to the file `lj-keys.005`; if they become unusable, you can delete this file to reset them to the defaults.

Some controls during game play are hard-coded to keyboard keys:

*   Esc pauses and resumes the game. Holding Esc for one second stops the game and goes to a result screen.
*   [ (left bracket) starts and stops demo recording, and ] (right bracket) starts and stops demo playback. An icon for stop, record, or play appears in the upper left corner of the window. The demo is saved to a file called `demo.ljm`. **Caution:** Demos recorded on one version of LOCKJAW may not play correctly on another version. Recording another demo will overwrite the last demo, so make sure to rename the demo when you record a good one.

In the menus, Esc acts as Rotate Left, and Enter acts as Rotate Right. In all screens, Print Screen (F13) saves a copy of the display to the file `ljsnap.bmp`.

### GBA and DS

Controls in the Game Boy Advance and Nintendo DS versions are hardcoded as follows:

<dl>

<dt>← Shift left</dt>

<dd>Left on Control Pad</dd>

<dt>→ Shift right</dt>

<dd>Right on Control Pad</dd>

<dt>↓ Soft drop</dt>

<dd>Down on Control Pad</dd>

<dt><u>↓</u> Hard drop</dt>

<dd>Up on Control Pad</dd>

<dt>↰ Rotate left</dt>

<dd>B Button</dd>

<dt>↱ Rotate right</dt>

<dd>A Button</dd>

<dt>↖ Hold piece</dt>

<dd>L Button</dd>

</dl>

In the options menu, the B and A Buttons change screens, and the Start Button starts the game. Pressing A on the last screen of options also starts the game. Start pauses and resumes the game.

### Movement features

Unlike some other <abbr title="Soviet Mind Game">S.M.G.</abbr> implementations, LOCKJAW features Initial Actions. Holding a rotate button while a tetromino enters the playfield will cause the action to be performed the moment the tetromino enters. This is important for fast play. In addition, the hold key works at any time; if a piece is not falling yet, it will swap the hold piece with the next piece.

LOCKJAW also implements the so-called [Super Rotation System (SRS)](http://www.tetrisconcept.com/tetriswiki/SRS), which allows a tetromino to rotate around obstacles for more mobility across the pile. Some players abuse SRS by [rotating a piece in place repeatedly](http://infinitespin.ytmnd.com/), but this will result in poorer scores in timed gimmicks. To replace this behavior with the "Sega style" behavior used in Arika's _Tetris The Grand Master 3: Terror-Instinct_, change the [Options described below](#options).

## <a name="scoring">Scoring</a>

LOCKJAW allows the player to choose among several scoring methods. The terms "single", "double", "triple", and "home run" refer to clearing 1, 2, 3, or 4 lines with one tetromino. (A "home run" is called a "tetris" in some other games.) "T-spin" means rotating a T piece as it lands to fit into a tight space.

In LJ and TDS, making a bonus line clear when your last line clear was also bonus ("back-to-back homer" or "back-to-back T-spin") will produce extra points. Making a T-spin that does not clear a line has no effect on bonus state.

### LJ

As you clear lines, you also produce garbage that depends on the number of lines that you clear with a single tetromino. In single player mode, you earn 100 points per line cleared and 200 points per line of garbage, and in multiplayer mode (which is not yet implemented), the garbage will push up the blocks in another player's well.

<table summary="first row: column headings"><caption>LJ scoring method</caption>

<thead>

<tr>

<th></th>

<th>Lines</th>

<th>Garbage</th>

<th>Score</th>

<th>Bonus</th>

</tr>

</thead>

<tbody>

<tr>

<td>1 line</td>

<td>1</td>

<td>0</td>

<td>100</td>

<td>No</td>

</tr>

<tr>

<td>2 lines</td>

<td>2</td>

<td>1</td>

<td>400</td>

<td>No</td>

</tr>

<tr>

<td>3 lines</td>

<td>3</td>

<td>2</td>

<td>700</td>

<td>No</td>

</tr>

<tr>

<td>4 lines</td>

<td>4</td>

<td>4</td>

<td>1200</td>

<td>Yes</td>

</tr>

<tr>

<td>1 line T-spin</td>

<td>1</td>

<td>2</td>

<td>500</td>

<td>Yes</td>

</tr>

<tr>

<td>2 lines T-spin</td>

<td>2</td>

<td>4</td>

<td>1000</td>

<td>Yes</td>

</tr>

<tr>

<td>3 lines T-spin</td>

<td>3</td>

<td>6</td>

<td>1500</td>

<td>Yes</td>

</tr>

<tr>

<td>Back-to-back bonus</td>

<td></td>

<td>1</td>

<td>200</td>

<td>Yes</td>

</tr>

</tbody>

</table>

### TNT64

Based on the Fibonacci sequence.

<table summary="first row: column headings"><caption>TNT scoring method</caption>

<thead>

<tr>

<th>Lines</th>

<th>Score</th>

</tr>

</thead>

<tbody>

<tr>

<td>1</td>

<td>100</td>

</tr>

<tr>

<td>2</td>

<td>200</td>

</tr>

<tr>

<td>3</td>

<td>300</td>

</tr>

<tr>

<td>4</td>

<td>500</td>

</tr>

<tr>

<td>5</td>

<td>800</td>

</tr>

<tr>

<td>6</td>

<td>1300</td>

</tr>

<tr>

<td>7</td>

<td>2100</td>

</tr>

</tbody>

</table>

### Hotline

Only lines cleared on specific rows are worth points. There is no back-to-back bonus nor T-spin bonus.

<table summary="first row: column headings"><caption>Hotline scoring method</caption>

<tbody>

<tr>

<th>Row</th>

<th>Score</th>

</tr>

</tbody>

<tbody>

<tr>

<td>5</td>

<td>100</td>

</tr>

<tr>

<td>10</td>

<td>200</td>

</tr>

<tr>

<td>14</td>

<td>300</td>

</tr>

<tr>

<td>17</td>

<td>400</td>

</tr>

<tr>

<td>19</td>

<td>500</td>

</tr>

<tr>

<td>20</td>

<td>600</td>

</tr>

</tbody>

</table>

### TDS

Each line clear score is multiplied by a section number, computed by dividing the total number of lines cleared before this line by 10 and adding 1\. Thus, lines cleared later in the game when gravity is faster are worth more than lines cleared earlier. In TDS, the section number stops increasing after 190 lines.

<table summary="first row: column headings"><caption>TDS scoring method</caption>

<thead>

<tr>

<th></th>

<th>Lines</th>

<th>Score</th>

<th>Bonus</th>

</tr>

</thead>

<tbody>

<tr>

<td>1 line</td>

<td>1</td>

<td>100 * section</td>

<td>No</td>

</tr>

<tr>

<td>2 lines</td>

<td>2</td>

<td>300 * section</td>

<td>No</td>

</tr>

<tr>

<td>3 lines</td>

<td>3</td>

<td>500 * section</td>

<td>No</td>

</tr>

<tr>

<td>4 lines</td>

<td>4</td>

<td>800 * section</td>

<td>Yes</td>

</tr>

<tr>

<td>0 lines T-spin (wall kick)</td>

<td>0</td>

<td>100 * section</td>

<td></td>

</tr>

<tr>

<td>0 lines T-spin (no wall kick)</td>

<td>0</td>

<td>400 * section</td>

<td></td>

</tr>

<tr>

<td>1 line T-spin (wall kick)</td>

<td>1</td>

<td>200 * section</td>

<td>Yes</td>

</tr>

<tr>

<td>1 line T-spin (no wall kick)</td>

<td>1</td>

<td>800 * section</td>

<td>Yes</td>

</tr>

<tr>

<td>2 lines T-spin</td>

<td>2</td>

<td>1200 * section</td>

<td>Yes</td>

</tr>

<tr>

<td>3 lines T-spin</td>

<td>3</td>

<td>1600 * section</td>

<td>Yes</td>

</tr>

<tr>

<td>Back-to-back bonus</td>

<td></td>

<td>50% more</td>

<td>Yes</td>

</tr>

</tbody>

</table>

### NES

Each line clear is multiplied by the section number, as in TDS. The section number does not stop increasing. There is no back-to-back bonus nor T-spin bonus.

<table summary="first row: column headings"><caption>NES scoring method</caption>

<thead>

<tr>

<th>Lines</th>

<th>Score</th>

</tr>

</thead>

<tbody>

<tr>

<td>1</td>

<td>40 * section</td>

</tr>

<tr>

<td>2</td>

<td>100 * section</td>

</tr>

<tr>

<td>3</td>

<td>300 * section</td>

</tr>

<tr>

<td>4</td>

<td>1200 * section</td>

</tr>

</tbody>

</table>

### Drop scoring

Some games award extra points every time the tetromino lands if the player used soft drop or hard drop. LOCKJAW can simulate these.

<dl>

<dt>None</dt>

<dd>Award no points.</dd>

<dt>Continuous</dt>

<dd>Award 1 point per row for hard drops. Award 1 point per row between when a soft drop starts when the piece lands. A soft drop that is started, then stopped, then started again, will award points only for the last soft drop.</dd>

<dt>Drop</dt>

<dd>Award 1 point per row for hard drops and for soft drops, whether or not they are interrupted.</dd>

<dt>Soft x1 Hard x2</dt>

<dd>Award 1 point per row for soft drops and 2 points per row for hard drops.</dd>

</dl>

## <a name="gimmicks">Gimmicks</a>

Gimmicks are the game modes in LOCKJAW.

<dl>

<dt>Marathon</dt>

<dd>The game gets faster and faster as each tetromino enters the well. Play until you top out. Some people play for lines; others play for points.</dd>

<dt>40 lines</dt>

<dd>Play until you clear 40 lines, or until you top out, whatever comes first. The author's record is 1:00.70.</dd>

<dt>180 seconds</dt>

<dd>Play for three minutes, or until you top out, whatever comes first.</dd>

<dt>Vs. w/Items</dt>

<dd>This mode is a joke. After the first 7 pieces you get random starting orientations, no rotation, and hidden next pieces, and the speed goes to 1G. Every time you're about to get an I tetromino, either you get 2 lines of garbage or the columns of blocks in the well are shuffled. It is debatable whether this mode is even playable. This mode exists primarily as a criticism of a similar mode in the game _Tetris DS_.</dd>

<dt>Baboo!</dt>

<dd>Standard S.M.G. with zero gravity, ending after 300 keypresses. Some players recommend practicing Baboo!, claiming that the way to fast play in other gimmicks involves using as few keystrokes as possible to place each tetromino.</dd>

<dt>Play .ljm</dt>

<dd>Allows the player to select a demo using a file chooser and then play it back. Present only in PC builds.</dd>

</dl>

## <a name="options">Options</a>

In Tetris, rules change you. But in LOCKJAW, **you** change the rules:

### Definitions

<dl>

<dt>Frame</dt>

<dd>The fundamental time unit of LOCKJAW. There are always 60 frames per second, regardless of the refresh rate of the attached monitor.</dd>

<dt>G</dt>

<dd>1G is a velocity of 1 cell per frame, or 60 cells per second. "20G" means that tetrominoes fall through the entire height of the well in one frame.</dd>

<dt>Hertz (Hz)</dt>

<dd>1 Hz is a rate of 1 event per second.</dd>

</dl>

### Well rules

<dl>

<dt>Well width</dt>

<dd>Standard S.M.G. uses 10 columns; it can be decreased to 4.</dd>

<dt>Well height</dt>

<dd>Standard S.M.G. uses 20 rows; it can be decreased to 8.</dd>

<dt>Speed curve</dt>

<dd>This controls how fast tetrominoes fall:

*   Exponential is the familiar behavior where the game speeds up gradually as pieces enter the well.
*   Rhythm starts at 20G; if your play speed drops below the stated level, tetrominoes will lock on their own. This level increases by 10 tetrominoes per minute every 64 beats.
*   Master and Death are similar to Exponential and Rhythm but resemble some Japanese S.M.G. implementations: every line cleared advances the level by 1, and except at levels 99, 199, 299, etc., so does every tetromino dropped. (Death is like starting Master at level 600, where blocks fall at 20G.)
*   NES and Game Boy speed curves approximate the behavior of the classic 8-bit games published by Nintendo.
*   In Zero, tetrominoes do not fall on their own. It's pointless for setting endurance records but useful for practicing tricky stacking methods.
*   Rhythm Zero is like Rhythm without the 20G.

</dd>

<dt>Max entry delay</dt>

<dd>When set greater than 0, there is a brief delay before each tetromino enters the playfield, and another delay when one or more lines are cleared. (So if you want fast play, make more lines at once.) During entry delay, sometimes called "ARE", the player can hold the next piece, rotate the next piece (if turned on), and charge up the autorepeat of sideways movement. This delay is considered a "maximum", as it decreases over the course of the game in Master and Death speed curves. Caution: When entry delay is on, hard drop also functions as an Initial Action.</dd>

<dt>Randomizer</dt>

<dd>This selects the function used to generate the pseudorandom stream of tetrominoes:

*   The default is the 7-piece bag, meaning that the tetrominoes are dealt in sets of 7, where all 7 different tetrominoes will be dealt before the next 7.
*   The 14-piece bag waits for each tetromino to be dealt twice before reshuffling.
*   The 6-piece bag is one stick short of a bundle because it never generates an I tetromino. It's useful for practicing "push" and T-spins.
*   Move to Back chooses one of the front three and moves it to the back of the set of 7.
*   History 6 Rolls is similar to Move to Back but sometimes (3.5%) generates repeats of recent tetrominoes.
*   Memoryless has a more or less equal probability of choosing each tetromino at any given time, as seen in most <abbr title="Soviet Mind Game">S.M.G.</abbr> implementations (including Tetris brand games from before 2001).
*   SZSZ is a "joke" randomizer, equivalent to Memoryless but dealing only the snake-shaped S and Z tetrominoes, and was used in a [proof that S.M.G. with the Memoryless randomizer cannot be played forever](http://www2.math.uic.edu/~burgiel/Tetris/explanation.html). (On the other hand, S.M.G. with hold piece and the bag randomizer can be played forever; here's [proof](http://www.tetrisconcept.com/tetriswiki/Playing_forever).)
*   The 10-piece bag contains smaller pieces in addition to tetrominoes.

</dd>

</dl>

### Movement rules

<dl>

<dt>Rotation system</dt>

<dd>In different rotation systems, tetrominoes may enter the field at different positions, and they behave differently when rotated against the walls and the other blocks. This option can be set to SRS (easiest), TOD M4 (like SRS in free space but less forgiving in wall kicks), Arika (like Sega with basic wall kicks), Sega or NES or Game Boy (no kicks), or Tengen (strict, with even stranger twists). Apart from one set of graphics for SRS and TOD M4 and one set for other systems, this option does not affect the colors of the tetrominoes; use the `skin.ini` commands to select replacement images that contain other colors. It also does not affect lockdown behavior.</dd>

<dt>Floor kicks</dt>

<dd>SRS and Arika rotation systems move a tetromino upward when a rotation would strike the floor or blocks under the tetromino. As this makes some tetrominoes easy to spin in place, this can detract from the immediacy of the game. This option limits the number of times each tetromino can be kicked upward.</dd>

<dt>Lockdown</dt>

<dd>Controls the interpretation of the delay between when each tetromino lands and when it locks. Classic disables the delay, as was the case in the oldest S.M.G. implementations; this makes slide moves more difficult. Entry reset allows only a constant amount of this delay per tetromino; instead of resetting, the lock timer pauses while the tetromino is falling. With Step reset, the delay resets every time the tetromino moves downward. With Move reset, the delay resets every time the tetromino moves at all. (Outside of Zero and Exponential speed curves, Classic behaves as Step reset.)</dd>

<dt>Lock delay</dt>

<dd>Controls the length of this delay. It can be set to a constant amount of time, or it can be controlled by the speed curve.</dd>

</dl>

### Line clear rules

<dl>

<dt>Line clear delay</dt>

<dd>Controls the length of the delay after a line clear. It can be set to a constant amount of time, or it can be controlled by the speed curve.</dd>

<dt>Clear gravity</dt>

<dd>Controls what happens to blocks after a line clear. Naive means that blocks move down by exactly the number of cleared lines below them, potentially causing floating blocks. In Sticky, tetrominoes stick together when they lock, and contiguous groups fall together. It's possible for a group to fall past the cleared lines, causing chain reactions. Sticky by color is similar to Sticky except only blocks of a single color stick together.</dd>

<dt>4x4 squares</dt>

<dd>Make a 4x4 block square out of four complete tetrominoes, and it will become a large square of solid gold or silver. Pieces that have been broken by a line clear cannot make squares and are thus drawn as garbage. A line containing a row of a silver square is worth 500 bonus points; a line containing a row of a gold square is worth 1000\. **Caution:** This mode is displayed incorrectly if the current [skin](#skin) lacks the appropriate `ljconn` file.</dd>

<dt>Scoring</dt>

<dd>Choose a scoring method for line clears, as described [above](#scoring).</dd>

<dt>T-spin detection</dt>

<dd>When turned off, T-spins are not rewarded. When set to Immobile, a line is a T-spin line if the tetromino that completes it could not have moved left, right, or up. When set to 3-corner T, a line is a T-spin line if all of the following are true:

*   The line is completed with a T tetromino.
*   At least 3 of the 4 boxes diagonal to the center of the tetromino are either the wall or occupied with a block.
*   The tetromino has not shifted sideways or down by one or more spaces since it was last rotated.

3-corner T no kick adds an additional restriction: The rotation itself did not move the tetromino.</dd>

<dt>Drop scoring</dt>

<dd>Choose a scoring method for drops, as described [above](#scoring).</dd>

<dt>Garbage</dt>

<dd>Pushes up the lines in your playfield. In Level 1 through 4, a simulated computer opponent occasionally sends you this many lines of garbage. In Home Run Derby, every line you clear other than with a home run or a T-spin gives you garbage. In Drill, the well is full of garbage except for the top two rows. Be prepared to rely on infinite spin for the first few lines until your skill improves.</dd>

</dl>

### Control options

<dl>

<dt>Sideways speed</dt>

<dd>This controls how fast the tetromino moves sideways. It can be set at Instant (tetromino reaches side instantly as soon as sideways delay expires) or 10 to 60 moves per second.</dd>

<dt>Max sideways delay</dt>

<dd>This controls the time the left or right key has to be held down before sideways movement begins. This can be set in 16.7 <abbr title="millisecond">ms</abbr> increments from 16.7 ms to 300 ms. This delay is considered a "maximum", as it decreases over the course of the game in Master and Death speed curves.</dd>

<dt>Initial sideways motion</dt>

<dd>When this is turned on, the tetromino can move sideways in the first frame that it appears. This affects mobility at very high speeds (e.g. Death or Rhythm).</dd>

<dt>Initial rotation</dt>

<dd>When this is turned on, the player can rotate a piece before it enters by holding down a rotate button.</dd>

<dt>Allow diagonal motion</dt>

<dd>When this is turned off, the Left, Right, Up, and Down keys will be ignored if Left or Right is pressed at the same time as Up or Down. Some players who use joysticks or gamepads claim that such "4-way" logic reduces misdrops. Keyboard players should leave it turned on, as it is necessary for "Lock on release" to work properly.</dd>

<dt>Soft drop speed</dt>

<dd>Controls how fast the tetromino moves downward in a soft drop. Can be set to 1G, 1/2G, or 1/3G.</dd>

<dt>Soft drop</dt>

<dd>Controls how tetrominoes behave when they land after being soft dropped with the Down key. "Lock" means that they will lock instantly. "Slide" allows the player to place a tetromino under an overhang in one smooth motion (move down, move sideways, then press Up to lock). "Lock on release" is similar to "Slide" but locks when the player lets go of Down.</dd>

<dt>Hard drop</dt>

<dd>Controls how tetrominoes behave when they land after being hard dropped with the Up key. "Lock" means that hard-dropped tetrominoes will lock instantly. "Slide" allows the player to place a tetromino under an overhang in one smooth motion (move down, move sideways, then press Down to lock). "Lock on release" is similar to "Slide" but locks when the player lets go of Up.</dd>

</dl>

### Display options

<dl>

<dt>Next pieces</dt>

<dd>Controls the number of previewed pieces displayed next to the well.</dd>

<dt>Next above shadow</dt>

<dd>Controls the number of previewed pieces displayed above the shadow.</dd>

<dt>Shadow</dt>

<dd>Controls how the falling tetromino and its shadow are displayed. It can show the falling tetromino along with a transparent colored shadow, opaque colored shadow, or colorless shadow. It's also possible to hide the shadow entirely or (for experts only) hide both the shadow and the falling tetromino.</dd>

<dt>Hide blocks in well</dt>

<dd>When turned on, hides the blocks that have locked down in the well. Useful for practicing for "invisible challenge" in another game.</dd>

<dt>Smooth gravity</dt>

<dd>When this is turned off, tetrominoes fall in units of one block. When turned on, they fall pixel by pixel, and only the bottom edge of each block in a tetromino is tested for collision against the blocks in the well.</dd>

<dt>Trails</dt>

<dd>When this is turned on, hard drops and fast piece gravity cause the tetromino to make a trail as it falls. It makes players feel faster at the game.</dd>

<dt>Playfield position</dt>

<dd>This moves all gameplay graphics to the left, middle, or right of the screen. Some background images work better with one setting than with another.</dd>

<dt>Next piece position</dt>

<dd>Controls where the next tetrominoes are displayed. Up to eight can fit to the right of the well, or up to 3 can fit above the well.</dd>

<dt>Pause on task switch</dt>

<dd>When turned on, automatically pauses the game when the player switches to another window.</dd>

<dt>Record all games</dt>

<dd>Automatically records each game to <tt>demo.ljm</tt>. Remember to rename your record-setting replays.</dd>

<dt>Display (after restart)</dt>

<dd>Choose full screen or windowed mode. Changes take effect the next time you start the program.</dd>

</dl>

These options are saved to `lj.ini` every time you exit the options screen. Other options not shown are also saved to `lj.ini` and can be changed with a text editor.

## <a name="discussion">Discussion</a>

If you want to discuss the game, you can do so at [TetrisConcept.com forum](http://www.tetrisconcept.com/forum/viewtopic.php?t=100).

## <a name="legal">Legal</a>

<div style="border: 1px solid black; padding: 1em 2em">

Copyright 2006 Damian Yerrick <[tepples+lockjaw (at) spamcop (full stop) net](http://www.pineight.com/contact/)>. This manual is under the following license:

This work is provided 'as-is', <span style="font-variant: small-caps">without any express or implied warranty.</span> In no event will the authors be held liable for any damages arising from the use of this work.

Permission is granted to anyone to use this work for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1.  The origin of this work must not be misrepresented; you must not claim that you wrote the original work. If you use this work in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2.  Altered source versions must be plainly marked as such, and must not be misrepresented as being the original work.
3.  This notice may not be removed or altered from any source distribution.

The term "source" refers to the preferred form of a work for making changes to it.

</div>

The LOCKJAW software described by this manual is distributed under the [GNU General Public License, version 2 or later](http://www.gnu.org/copyleft/gpl.html), <span style="font-variant: small-caps">with absolutely no warranty.</span> See `COPYING.txt` for details.

<a name="trademarks">*</a> Tetris is a trademark of The Tetris Company. The Software is not sponsored or endorsed by Alexey Pajitnov, Elorg, Cloudmakers, Microsoft, Nintendo, or The Tetris Company.

</div>
