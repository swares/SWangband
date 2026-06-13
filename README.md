# SWangband

SWangband is a modified version of Angband 4.2.6 with four gameplay additions:
Living Stores, Second Wind, Difficulty Presets, and Danger Telemetry.

---

## What's Different

### Living Stores
Shop inventories scale with your current dungeon depth. The deeper you've
ventured, the better the stock you'll find when you return to town. Items
rotate periodically — if you pass on something, it may be gone next visit.
This makes town trips more meaningful and rewards deeper exploration.

### Second Wind
Once per game, instead of dying you receive a second chance: your HP is
restored to a fraction of maximum and you continue from where you fell.
The sidebar shows your remaining Second Wind charges ("Wind: 1" when
available, blank when spent). Enable or disable this at character creation
with the **Second Wind** birth option.

### Difficulty Presets
At character creation you can choose a difficulty preset that adjusts
multiple challenge parameters at once — monster speed, item quality, and
experience penalties. Presets range from Apprentice (forgiving) to
Ironman (brutal). These are birth options and cannot be changed after
the game begins.

### Danger Telemetry
A threat meter appears in the sidebar showing the danger level of monsters
currently visible on the map. The meter updates as monsters move in and out
of view and updates when your HP changes. Levels are:

| Label   | Meaning                                      |
|---------|----------------------------------------------|
| SAFE    | No dangerous monsters in sight               |
| CAUTION | Monsters present but manageable              |
| DANGER  | Significant threat — consider retreating     |
| LETHAL  | Extreme danger — immediate action required   |

Enable or disable the threat meter with the **Show threat meter** option
(`=` → Birth options → Show threat meter).

---

# Angband 4.2.6

<p align="center">
  <img src="screenshots/title.png" width="425"/>
  <img src="screenshots/game.png" width="425"/>
</p>

Angband is a graphical dungeon adventure game that uses textual characters to
represent the walls and floors of a dungeon and the inhabitants therein, in the
vein of games like NetHack and Rogue. If you need help in-game, press `?`.

- **Installing Angband:** See the [Official Website](https://angband.github.io/angband/) or [compile it yourself](https://angband.readthedocs.io/en/latest/hacking/compiling.html).
- **How to Play:** [The Angband Manual](https://angband.readthedocs.io/en/latest/)
- **Getting Help:** [Angband Forums](https://angband.live/forums/)

Enjoy!

-- The Angband Dev Team
